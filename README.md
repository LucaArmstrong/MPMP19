# MPMP19

My solution to Matt Parker's Maths Puzzles 19 Challenge - Find numbers n such that the sum of the squares of the first n primes is divisible by n - sequence A111441 in the OEIS.

# Method

This program makes use of Kim Walisch's primesieve library. The program is broadly split up into two sections, sequencing and modulus operations.

In the sequencing section, primes are generated using primesieve and their squares are added to an array called `mod data`. A nice fact about this problem is 

If $S(n)$ denotes the sum of the squares of the first $n$ primes, then the goal is to try and find values of $n$ such that $S(n) \equiv 0 \mod n$. 

A nice trick we can use is that every prime $p$ other than $2$ and $3$ has the property that $p^2\equiv 1 \mod 6$. Using this, it isn't too hard to see that $S(n) \equiv n + 11 \mod 6$ for every $n\ge 2$. Thus, if $n\mid S(n)$, then $n$ must be coprime to $6$, or $n\equiv 1,5 \mod 6$. This immediately eliminates two thirds of the possible modulus checks. 




# Verification

This program doesn't use any verification to test that the square sums and the terms calculated are indeed valid. One approach you could use is an extended Meissel-Lehmer type algorithm to compute the sum of the squares of the primes up to some number $n$ in $O(n^{2/3+\epsilon})$ time. Doing this every so often would ensure the numbers being used are correct while not increasing the runtime that much. Unfortunately, I do not have the expertise to implement such an algorithm, but this is definitely something that could be added in the future.


# Results

I ran this program on my AMD Ryzen 3500U processor using 8 logical threads and 1GB of RAM and the results are shown here:

| n  | a(n)    | Time Taken  |   |   |   |
|----|--------------|-------------|---|---|---|
| 10 | 542474231   | ~ 1 second  |   |   |   |
| 11 | 1139733677   | ~ 2 seconds |   |   |   |
| 12 | 51283502951  | ~ 2 minutes 56 seconds |   |   |   |
| 13 | 230026580777 | ~ 19 minutes 7 seconds |   |   |   |

On my laptop, it would take around 3 days to reach term a(14) which is why I didn't bother extending this list. 

In the long run, the modulus checks end up taking a smaller and smaller proportion of the run time whereas most of the time gets spent on generating all the primes. This should make sense, since the probability of an integer being prime decreases as the limit increases. 

The term a11 is reached in under 10 seconds 
