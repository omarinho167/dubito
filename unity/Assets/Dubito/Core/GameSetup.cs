using System;
using System.Collections.Generic;

namespace Dubito.Core
{
    public static class GameSetup
    {
        public static GameState Create(GameConfig config, IRandomSource random)
        {
            if (config == null) throw new ArgumentNullException(nameof(config));
            if (random == null) throw new ArgumentNullException(nameof(random));

            var deck = new List<Card>(Deck.CreateStandard());
            Shuffle(deck, random);
            var order = new List<int>(config.PlayerCount);
            var hands = new List<List<Card>>(config.PlayerCount);
            for (int id = 0; id < config.PlayerCount; id++)
            {
                order.Add(id);
                hands.Add(new List<Card>());
            }
            Shuffle(order, random);

            int cursor = 0;
            int baseCount = deck.Count / config.PlayerCount;
            for (int round = 0; round < baseCount; round++)
            foreach (int id in order)
                hands[id].Add(deck[cursor++]);

            // Sample without replacement, independently of the seating shuffle.
            // A partial Fisher-Yates shuffle gives each remaining card a distinct recipient.
            var recipients = new List<int>(order);
            int remainder = deck.Count % config.PlayerCount;
            for (int i = 0; i < remainder; i++)
            {
                int selected = i + Draw(random, recipients.Count - i);
                Swap(recipients, i, selected);
                hands[recipients[i]].Add(deck[cursor++]);
            }
            return new GameState(config, order, hands);
        }

        private static void Shuffle<T>(List<T> items, IRandomSource random)
        {
            for (int i = items.Count - 1; i > 0; i--)
                Swap(items, i, Draw(random, i + 1));
        }

        private static int Draw(IRandomSource random, int bound)
        {
            int value = random.Next(bound);
            if (value < 0 || value >= bound)
                throw new InvalidOperationException("Random source returned a value outside its requested range.");
            return value;
        }

        private static void Swap<T>(List<T> items, int a, int b)
        {
            T item = items[a];
            items[a] = items[b];
            items[b] = item;
        }
    }
}
