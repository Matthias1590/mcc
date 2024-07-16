from __future__ import annotations

def indent(code: str) -> str:
    return "\n".join(["    " + line for line in code.split("\n")])

class Debug:
    def __repr__(self) -> str:
        return f"{self.__class__.__name__}({', '.join(f'{k}={v!r}' for k, v in self.__dict__.items())})"

class SymbolTable(Debug):
    def __init__(self, parent: SymbolTable | None = None) -> None:
        super().__init__()
        self.parent = parent
        self.symbols = {}
    
    def add(self, name: str, type: Type) -> None:
        if name in self.symbols:
            print(f"redeclaration of {name!r}")
            exit(1)

        self.symbols[name] = type
    
    def expect(self, name: str) -> Type:
        if name not in self.symbols:
            if not self.parent:
                raise Exception(f"variable {name} does not exist")
            
            return self.parent.expect(name)

        return self.symbols[name]

    def sub_table(self) -> SymbolTable:
        return SymbolTable(self)

SYMBOL_TABLE = SymbolTable()

class QbeVar:
    def __init__(self, state: QbeState, name: str) -> None:
        super().__init__()
        self.state = state
        self.name = name
    
    def free(self) -> None:
        self.state.vars.remove(self.name)

    def __repr__(self) -> str:
        return repr(self.name)

class QbeState(Debug):
    def __init__(self) -> None:
        super().__init__()
        self.vars = set()
        self.total = 0

    def create_var(self) -> QbeVar:
        var = QbeVar(self, self.total)
        self.vars.add(var.name)
        self.total += 1
        return var

class Result(Debug):
    def __init__(self, type: Type, returned: Type) -> None:
        super().__init__()
        self.type = type
        self.returned = returned

class Node(Debug):
    def check(self, table: SymbolTable) -> Result:
        raise NotImplementedError(self)

    def to_qbe(self, state: QbeState) -> QbeVar:
        raise NotImplementedError(self)

class FunctionDecl(Node):
    def __init__(self, name: str, return_type: Type, params: list[Param], body: Node) -> None:
        super().__init__()
        self.name = name
        self.return_type = return_type
        self.params = params
        self.body = body
    
    def check(self, table: SymbolTable) -> Result:
        table.add(self.name, FunctionType(self.return_type, [param.type for param in self.params]))

        sub_table = table.sub_table()
        for param in self.params:
            sub_table.add(param.name, param.type)
        body_res = self.body.check(sub_table)
        if not body_res.returned and not self.return_type:
            return
        if not body_res.returned:
            raise Exception(f"not returning a value from function {self.name!r}")
        if not body_res.returned.can_be(self.return_type):
            raise Exception(f"returning {self.body.check(sub_table)}, expected {self.return_type}")

    def to_qbe(self, state: QbeState) -> QbeVar:
        print("function ", end="")
        self.return_type.to_qbe(state)
        print(f" ${self.name}(", end="")
        for i in range(len(self.params)):
            if i != 0:
                print(", ", end="")
            self.params[i].to_qbe(state)
        print(") {", end="")
        print("\n@start\n", end="")
        self.body.to_qbe(state)
        print("}", end="")

class Type(Node):
    def can_add(self, other: Type) -> bool:
        raise NotImplementedError(self)

    def get_add_res_type(self, other: Type) -> Type:
        raise NotImplementedError(self)

    def can_be(self, other: Type) -> bool:
        raise NotImplementedError(self)

class FunctionType(Type):
    def __init__(self, return_type: Type, param_types: list[Type]) -> None:
        super().__init__()
        self.return_type = return_type
        self.param_types = param_types

    def can_add(self, other: Type) -> bool:
        return False

    def can_be(self, other: Type) -> bool:
        return isinstance(other, FunctionType) and self.return_type.can_be(other.return_type)

class Primitive(Type):
    pass

class Int(Primitive):
    def can_add(self, other: Type) -> bool:
        return isinstance(other, Int)

    def get_add_res_type(self, other: Type) -> Type:
        if isinstance(other, Int):
            return Int()

    def can_be(self, other: Type) -> bool:
        return isinstance(other, Int)

    def to_qbe(self, state: QbeState) -> QbeVar:
        print("w", end="")

class Param(Node):
    def __init__(self, type: Type, name: str) -> None:
        super().__init__()
        self.type = type
        self.name = name

    def to_qbe(self, state: QbeState) -> QbeVar:
        self.type.to_qbe(state)
        print(f" %{self.name}", end="")

class IfStmt(Node):
    def __init__(self, if_: Node, else_: Node) -> None:
        super().__init__()
        self.if_ = if_
        self.else_ = else_

    def check(self, table: SymbolTable) -> Result:
        returned = None

        if_res = self.if_.check(table)
        else_res = self.else_.check(table)

        if if_res is not None and else_res is not None and if_res.returned.can_be(else_res.returned):
            returned = if_res.returned
        return Result(None, returned)

class ReturnStmt(Node):
    def __init__(self, value: Node) -> None:
        super().__init__()
        self.value = value

    def check(self, table: SymbolTable) -> Result:
        if not self.value:
            return Result(None, None)
        return Result(None, self.value.check(table).type)

    def to_qbe(self, state: QbeState) -> QbeVar:
        if not self.value:
            return print("ret\n", end="")

        var = self.value.to_qbe(state)
        print(f"ret %{var}\n", end="")
        var.free()

class Variable(Node):
    def __init__(self, name: str) -> None:
        super().__init__()
        self.name = name
    
    def check(self, table: SymbolTable) -> Result:
        self.type = table.expect(self.name)
        return Result(self.type, None)

    def to_qbe(self, state: QbeState) -> QbeVar:
        var = state.create_var()
        print(f"%{var} =", end="")
        self.type.to_qbe(state)
        print(f" copy %{self.name}")
        return var

class AddExpr(Node):
    def __init__(self, left: Node, right: Node) -> None:
        super().__init__()
        self.left = left
        self.right = right

    def check(self, table: SymbolTable) -> Result:
        left_type = self.left.check(table)
        right_type = self.right.check(table)

        if not left_type.type.can_add(right_type.type):
            raise Exception(f"cant add {left_type.type} and {right_type.type}")

        self.type = left_type.type.get_add_res_type(right_type.type)
        return Result(self.type, None)

    def to_qbe(self, state: QbeState) -> QbeVar:
        left = self.left.to_qbe(state)
        right = self.right.to_qbe(state)

        top = state.create_var()
        print(f"%{top} =", end="")
        self.left.type.get_add_res_type(self.right.type).to_qbe(state)
        print(f" add %{left}, %{right}\n", end="")

        left.free()
        right.free()
        return top

ast = FunctionDecl("add", Int(), [Param(Int(), "a"), Param(Int(), "b")], ReturnStmt(AddExpr(Variable("a"), Variable("b"))))

# print(ast)

ast.check(SYMBOL_TABLE)

# print(SYMBOL_TABLE)

state = QbeState()
ast.to_qbe(state)
print()
