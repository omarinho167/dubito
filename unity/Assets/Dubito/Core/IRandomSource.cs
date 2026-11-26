using System;

namespace Dubito.Core
{
    public interface IRandomSource
    {
        // Uniform integer in [0, exclusiveMax). The caller supplies a positive bound.
        int Next(int exclusiveMax);
    }

    public sealed class SystemRandomSource : IRandomSource
    {
        private readonly Random random;

        public SystemRandomSource() { random = new Random(); }
        public SystemRandomSource(int seed) { random = new Random(seed); }
        public int Next(int exclusiveMax) => random.Next(exclusiveMax);
    }
}
