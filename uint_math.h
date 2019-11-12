mp::uint256_t uipowmod(mp::uint256_t basearg, mp::uint256_t exp, mp::uint256_t mod)
{
    mp::uint512_t base = basearg;
    mp::uint512_t result = 1;
    for (;;)
    {
        if (exp & 1)
        {
            result *= base;
            if (result > mod)
                result %= mod;
        }
        exp >>= 1;
        if (exp==0)
            break;
        base *= base;
        if (base > mod)
            base %= mod;
    }
    return (mp::uint256_t)result;
}

