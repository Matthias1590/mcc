int main(int argc) {
    int x;
    int y = x * 2;
    {
        int argc = 2;
        y = 9;
    }
    return argc + y;
}
