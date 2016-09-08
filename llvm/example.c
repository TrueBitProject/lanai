unsigned mod(unsigned a, unsigned b)
{
    while (a >= b) a -= b;
    return a;
}

int prime(int n)
{
   int i = 2;
   int c;
   int count = 0;
   int primes[200];
   int lastprime = 2;
 
   while (count < n)
   {
      for ( c = 2 ; c <= i - 1 ; c++ )
         if ( mod(i, c) == 0 )
            break;
      if ( c == i )
        primes[count++] = lastprime = i;
      i++;
   }
   return lastprime;
}

int simple(int a)
{
    return a + 20;
}

int compare(unsigned a, unsigned b)
{
    return a >= b;
}

int select(int a, int b)
{
    if (a < b)
        return 1;
    else
        return 2;
}

int fill()
{
    int x[20];
    for (int i = 0; i < 20; i++)
        x[i] = i;
    return x[19];
}

int main()
{
//    return simple(20);
//    return mod(0x24, 3);
    return prime(20);
//      return fill();
}
