using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace HammingMultiIndexTestDLL
{
    class Program
    {
        const uint ITEM_SIZE = 32;
        const uint BATCH_SIZE = 100000;
        const uint BATCH_NUMBER = 1000;
        const uint QUERY_NUMBER = 10;
        const float RANGE = 0.1f;

        /// <summary>
        /// Random number generator.
        /// </summary>
        static Random random = new Random();

        /// <summary>
        /// Returns random array of specified length.
        /// </summary>
        static byte[] GetRandomArray(uint size)
        {
            byte[] result = new byte[size];
            random.NextBytes(result);
            return result;
        }

        /// <summary>
        /// Simple test for wrapper and DLL itself.
        /// </summary>
        static void Main(string[] args)
        {
            MultiIndex index = new MultiIndex(ITEM_SIZE);
            
            // Fill index.
            Console.WriteLine("Filling index...");
            uint counter = 0;
            for (uint i = 0; i < BATCH_NUMBER; i++)
            {
                List<uint> keys = new List<uint>();
                List<byte[]> items = new List<byte[]>();

                // Generate batch.
                for (uint j = 0; j < BATCH_SIZE; j++, counter++)
                {
                    keys.Add(counter);
                    items.Add(GetRandomArray(ITEM_SIZE));
                }

                // Add it to index.
                uint numberOfAddedItems = 0;
                index.AddItems(keys, items, ref numberOfAddedItems);
                Console.WriteLine("  Done: " + counter.ToString());
            }
            Console.WriteLine("Number of items now: " + index.GetNumberOfItems());

            // Process few search queries.
            Console.WriteLine("Querying...");
            for (uint i = 0; i < QUERY_NUMBER; i++)
            {
                byte[] query = GetRandomArray(ITEM_SIZE);
                List<KeyValuePair<uint, float>> keysAndDistances = new List<KeyValuePair<uint, float>>();
                index.DoRangeQueryOptimized(query, RANGE, keysAndDistances);
                Console.WriteLine("  Found: " + keysAndDistances.Count.ToString());
            }
            Console.WriteLine("Job`s done!");
        }
    }
}
