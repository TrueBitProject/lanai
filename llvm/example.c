int mod(int a, int b)
{
    while (a >= b) a -= b;
    return a;
}

int run()
{
   int n = 100;
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
