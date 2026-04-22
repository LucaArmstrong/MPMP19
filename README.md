# MPMP19

This is my solution to [Matt Parker's Maths Puzzles 19 Challenge](https://www.youtube.com/watch?v=tBXGIXEV7tI) — "Find numbers $n$ such that the sum of the squares of the first $n$ primes is divisible by $n$". Terms are listed as sequence A111441 on the OEIS.

# Method
Let $S(n)$ denote the sum of the squares of the first $n$ primes — the goal is to find values of $n$ such that $S(n) \equiv 0 \mod n$. A simple way to do this is to initialise $S=n=0$, then:

1. Get the next prime $p$ and add its square to $S$
2. Compute $S(n)\mod n$ — if zero then record $n$ as a term.
3. Repeat.

This is straightforward to implement, but inherently sequential. The aim of this method is to take advantage of parallel processing as much as possible. To do this, we split the computation into intervals, and further divide each interval into equal-sized segments (one per thread). Each interval is processed as follows: (note this is just an overview)

- **Pass 1:** Each thread generates the primes in its segment, sums their squares, and stores the prime gaps between consecutive primes.
- **Fixup:** Using the cumulative $S(n)$ and $n$ at the start of the interval, and the partial sums from each thread, the exact values of $S(n)$ and $n$ at the start of each thread are reconstructed.
- **Pass 2:** Each thread re-traverses its segment using the stored gaps, checks divisibility at each step, and records any terms found.

Although each range is processed twice, this is considerably faster than a serial approach because both passes are fully parallel, and the stored gaps allow the second pass to reconstruct primes cheaply without re-invoking the sieve.


# Implementation

This program is written in C++ and makes use of Kim Walisch's [primesieve](https://github.com/kimwalisch/primesieve/tree/master) library, which is the state of the art for fast prime generation. Originally I wrote the program in C, but found the usage of structs and methods to be much cleaner with C++. My experience with the language is fairly limited, but I tried my best to translate the code without compromising performance. I admit much of the inspiration came from reading through the primesieve code. OpenMP is heavily used, specifically for parallelising the prime generation and the modular arithmetic passes.

### Key Features and Optimisations

- Uses one `primesieve::iterator` per thread to sieve primes in parallel across disjoint ranges.
- Uses a custom 192-bit unsigned integer type to accumulate sums of prime squares. This extends the theoretical search limit of the program from roughly $10^{12}$ (using only 128-bit integers) to over $10^{18}$, at which point other factors in the program become more important.
- Prime gaps are stored rather than primes themselves, reducing the memory required by approximately 8x (most gaps fit into a single byte compared with 8 bytes for primes themselves). A variable length encoding handles the rare larger gaps with almost no overhead. 
- Uses a recurrence relation to update the quotient $Q = \lfloor S(n)/n\rfloor$ and remainder $R = S(n)\mod n$ incrementally as $n$ advances, avoiding 192-bit division entirely.
- Exploits the fact that terms can only occur when $n\equiv 1 \text{ or } 5\mod 6$ to eliminate two thirds of the division operations in the modular arithmetic loop.
- The length of each segment is estimated using a prime number theorem based approximation and a correction factor, which keeps the number of primes processed per interval close to the target number. Consequently, the amount of memory used is kept close to the specified amount `memory_mb`.

It is recommended that the program be compiled with the highest optimisation level set: `-O3 -march=native` if using GNU/Clang/ICC. MSVC is not supported as it lacks compiler support for 128-bit integer types.

# Verification

This program does not currently use any verification to test that the square sums and the terms calculated are indeed valid. The most effective method I can think of doing this would be modifying a combinatorial algorithm for counting primes to instead sum their squares. For instance, a modified version of the Deleglise-Rivat algorithm could compute values of $S(n)$ in just $O\left(n^{2/3} / \log^{4/3}(n)\right)$ steps, which is way faster than recomputing the entire sequence from scratch. It also means that if there did happen to be an error in the code, instead of repeating that error in another run, we'd have a completely different method of checking values. This means if both values match, we can be certain that the sequence values are correct. 

Doing this every so often would ensure the numbers being used are correct while not increasing the runtime significantly. However, implementing a modified Deleglise-Rivat correctly, with 192-bit arithmetic is non-trivial, and not something I will be attempting anytime soon. 

# Results

Tested on:

- **CPU:** AMD Ryzen 5 3500U (4 cores / 8 threads, boost ~3.7 GHz)
- **RAM:** 6 GB
- **OS:** Debian 13 (kernel 6.12)
- **Compiler:** GCC 14.2.0, `-O3 -march=native`

| n  | a(n)    | Time Taken  |
|----|--------------|-------------|
| 10 | 542,474,231   | ~ 1 second  |
| 11 | 1,139,733,677   | ~ 2 seconds |
| 12 | 51,283,502,951  | ~ 2 minutes 56 seconds |
| 13 | 230,026,580,777 | ~ 19 minutes 7 seconds |

I decided to stop the program when it hit $n=10^{12}$, but continuing this would yield terms a(14) and a(15) within roughly a week on this hardware. The performance seems to scale well with thread count (from what I've tested). In the long run, the modulus checks take up a smaller and smaller proportion of the run time whereas the prime generation dominates. This should make sense as primes get sparser, and the interval sizes are made larger accordingly.
