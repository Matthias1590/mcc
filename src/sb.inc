%ifndef SB_INC
%define SB_INC

struc struct_sb
    .capacity resq 1
    .length resq 1
    .data resq 1
endstruc

%ifndef SB_NO_FUNCS
extern sb_new
extern sb_free
extern sb_add
%endif ; SB_NO_FUNCS

%endif ; SB_INC
