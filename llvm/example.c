void run(char *input)
{
    int counter = 0;
    for (int counter = 0; input[counter] != 0; counter++)
        input[counter] = counter;
}
