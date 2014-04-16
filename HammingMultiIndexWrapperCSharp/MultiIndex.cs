using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Threading;
using System.IO;

namespace HammingMultiIndexTestDLL
{
    class MultiIndex
    {
        [DllImport("HammingMultiIndexDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "CreateDefaultIndexAndGetHandle", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr NativeCreateDefaultIndexAndGetHandle(
            uint itemBytesNumber);

        [DllImport("HammingMultiIndexDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "CreateIndexAndGetHandle", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr NativeCreateIndexAndGetHandle(
            uint itemBytesNumber,
		    uint bucketPageSize,
		    uint hashTableSize,
		    uint pageSize,
		    float bruteForceBound);

        [DllImport("HammingMultiIndexDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "GetNumberOfItems", CallingConvention = CallingConvention.Cdecl)]
        private static extern UInt32 NativeGetNumberOfItems(
            IntPtr handle);

        [DllImport("HammingMultiIndexDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "Clear", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeClear(
            IntPtr handle);

        [DllImport("HammingMultiIndexDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "AddItems", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeAddItems(
            IntPtr handle,
		    IntPtr keys,
		    IntPtr items,
		    uint numberOfItems, 
		    ref uint numberOfTrulyAdded);

        [DllImport("HammingMultiIndexDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "DoRangeQueryBruteForce", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeDoRangeQueryBruteForce(		
            IntPtr handle,
		    IntPtr query, 
		    float range,
		    ref IntPtr keysAndDistances,
		    ref uint numberOfResults);

        [DllImport("HammingMultiIndexDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "DoRangeQueryOptimized", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeDoRangeQueryOptimized(
            IntPtr handle,
		    IntPtr query, 
		    float range,
		    ref IntPtr keysAndDistances,
		    ref uint numberOfResults);

        [DllImport("HammingMultiIndexDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "DestroyHandle", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeDestroyHandle(
            IntPtr handle);

        [DllImport("HammingMultiIndexDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "FreeKeysAndDistances", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeFreeKeysAndDistances(
            IntPtr keysAndDistances);

        //////////////////////////////////////////////////////////////////////////

        /// <summary>
        /// Size of binary string.
        /// </summary>
        private uint m_ItemBytesNumber = 0;

        /// <summary>
        /// Handle to native MultiIndex instance.
        /// </summary>
        private IntPtr m_Handle = IntPtr.Zero;

        /// <summary>
        /// Lock object to create critical sections.
        /// </summary>
        private Object m_LockObject = new Object();

        /// <summary>
        /// Public constructor with default parameters.
        /// <param name="itemBytesNumber">number of bytes in items which shall be indexed.</param> 
        /// </summary>
        public MultiIndex(
            uint itemBytesNumber)
        {
            try
            {
                m_ItemBytesNumber = itemBytesNumber;
                m_Handle = NativeCreateDefaultIndexAndGetHandle(itemBytesNumber);
            }
            catch (Exception e)
            {
                Console.WriteLine("Error : " + e.Message);
                m_ItemBytesNumber = 0;
                m_Handle = IntPtr.Zero;
                throw;
            }
        }

        /// <summary>
        /// Public constructor with specified parameters.
        /// <param name="itemBytesNumber">number of bytes in items which shall be indexed.</param> 
        /// <param name="bucketPageSize">size of page inside bucket (larger page size cause less allocations but increase memory overhead and vice versa).</param> 
        /// <param name="hashTableSize">number of items in hash table (more items mean faster lookup, but more space required and vice versa).</param> 
        /// <param name="pageSize">number of items per page (larger page size cause less allocations but increase memory overhead and vice versa).</param> 
        /// </summary>
        public MultiIndex(
            uint itemBytesNumber,
            uint bucketPageSize,
            uint hashTableSize,
            uint pageSize,
            float bruteForceBound)
        {
            try
            {
                m_ItemBytesNumber = itemBytesNumber;
                m_Handle = NativeCreateIndexAndGetHandle(
                    itemBytesNumber,
                    bucketPageSize,
                    hashTableSize,
                    pageSize,
                    bruteForceBound);
            }
            catch 
            {
            	m_ItemBytesNumber = 0;
                m_Handle = IntPtr.Zero;
                throw;
            }
        }

        /// <summary>
        /// Public destructor.
        /// </summary>
        ~MultiIndex()
        {
            if (m_Handle == IntPtr.Zero)
            {
                throw new Exception("MultiIndex handle == nullptr!");
            }
            NativeDestroyHandle(m_Handle);
        }

        /// <summary>
        /// Returns number of items in index.
        /// </summary>
        public UInt32 GetNumberOfItems()
        {
            if (m_Handle == IntPtr.Zero)
            {
                throw new Exception("MultiIndex handle == nullptr!");
            }
            lock (m_LockObject)
            {
                return NativeGetNumberOfItems(m_Handle);
            }
        }

        /// <summary>
        /// Removes all items from index.
        /// </summary>
        public void Clear()
        {
            if (m_Handle == IntPtr.Zero)
            {
                throw new Exception("MultiIndex handle == nullptr!");
            }
            lock (m_LockObject)
            {
                NativeClear(m_Handle);
            }
        }

        /// <summary>
        /// Adds new items to index.
        /// <param name="keys">items unique identifiers. Keys don`t have to go one-after-another.</param>
        /// <param name="items">items one needs to add.</param>
        /// <param name="numberOfAddedItems">number of items which was truly added.</param>
        /// </summary>
        public void AddItems(
            List<uint> keys,
            List<byte[]> items,
            ref uint numberOfAddedItems)
        {
            numberOfAddedItems = 0;
            if (m_Handle == IntPtr.Zero)
            {
                throw new Exception("MultiIndex handle == nullptr!");
            }
            if (keys.Count != items.Count)
            {
                throw new Exception("Ivalid arguments keys.Count != items.Count!");
            }
            lock (m_LockObject)
            {
                // Check data.
                foreach (byte[] item in items)
                {
                    if (item.Length != m_ItemBytesNumber)
                    {
                        throw new Exception("Number of bytes in item is " + item.Length.ToString() + " but one expect " + m_ItemBytesNumber.ToString());
                    }
                }

                // Marshal data.
                uint numberOfItemsToAdd = (uint)Math.Min(keys.Count, items.Count);
                uint[] marshaledKeys = new uint[keys.Count];
                IntPtr[] marshaledItems = new IntPtr[items.Count];
                for (uint i = 0; i < numberOfItemsToAdd; i++)
                {
                    marshaledKeys[i] = keys[(int)i];
                    marshaledItems[i] = Marshal.AllocHGlobal((int)m_ItemBytesNumber);
                    Marshal.Copy(items[(int)i], 0, marshaledItems[i], (int)m_ItemBytesNumber);
                }

                // Create GCHandles.
                GCHandle keysHandle = GCHandle.Alloc(marshaledKeys, GCHandleType.Pinned);
                GCHandle itemsHandle = GCHandle.Alloc(marshaledItems, GCHandleType.Pinned);

                // Call native method.
                NativeAddItems(
                    m_Handle,
                    keysHandle.AddrOfPinnedObject(),
                    itemsHandle.AddrOfPinnedObject(),
                    numberOfItemsToAdd,
                    ref numberOfAddedItems);

                // Free GCHandles.
                keysHandle.Free();
                itemsHandle.Free();

                // Free.
                for (uint i = 0; i < numberOfItemsToAdd; i++)
                {
                    Marshal.FreeHGlobal(marshaledItems[i]);
                }
            }
        }

        /// <summary>
        /// Process query via linear scan.
        /// </summary>
        /// <param name="query">query vector.</param>
        /// <param name="range">range radius.</param>
        /// <param name="keysAndDistances">results will be places here.</param>
        public void DoRangeQueryBruteForce(
            byte[] query, 
            float range,
            List<KeyValuePair<uint, float>> keysAndDistances)
        {
            if (m_Handle == IntPtr.Zero)
            {
                throw new Exception("MultiIndex handle == nullptr!");
            }  
            lock (m_LockObject)
            {
                GCHandle queryHandle = GCHandle.Alloc(query, GCHandleType.Pinned);
                uint numberOfResults = 0;
                IntPtr results = IntPtr.Zero;
                NativeDoRangeQueryBruteForce(
                    m_Handle,
                    queryHandle.AddrOfPinnedObject(),
                    range,
                    ref results,
                    ref numberOfResults);
                queryHandle.Free();

                // Decode results.
                if (numberOfResults == 0)
                {
                    keysAndDistances.Clear();
                    return;
                }
                uint bufferSize = numberOfResults * (sizeof(uint) + sizeof(float));
                byte[] buffer = new byte[bufferSize];
                Marshal.Copy(results, buffer, 0, (int)bufferSize);
                using (MemoryStream memoryStream = new MemoryStream())
                {
                    using (BinaryReader reader = new BinaryReader(memoryStream))
                    {
                        for (uint i = 0; i < numberOfResults; i++)
                        {
                            uint key = reader.ReadUInt32();
                            float distance = reader.ReadSingle();
                            keysAndDistances.Add(new KeyValuePair<uint, float>(key, distance));
                        }
                    }
                }
               
                // Free.
                NativeFreeKeysAndDistances(results);
            }
        }

        /// <summary>
        /// Process query via linear scan.
        /// </summary>
        /// <param name="query">query vector.</param>
        /// <param name="range">range radius.</param>
        /// <param name="keysAndDistances">results will be places here.</param>
        public void DoRangeQueryOptimized(
            byte[] query,
            float range,
            List<KeyValuePair<uint, float>> keysAndDistances)
        {
            if (m_Handle == IntPtr.Zero)
            {
                throw new Exception("MultiIndex handle == nullptr!");
            }
            lock (m_LockObject)
            {
                GCHandle queryHandle = GCHandle.Alloc(query, GCHandleType.Pinned);
                uint numberOfResults = 0;
                IntPtr results = IntPtr.Zero;
                NativeDoRangeQueryOptimized(
                    m_Handle,
                    queryHandle.AddrOfPinnedObject(),
                    range,
                    ref results,
                    ref numberOfResults);
                queryHandle.Free();

                // Decode results.
                if (numberOfResults == 0)
                {
                    keysAndDistances.Clear();
                    return;
                }
                uint bufferSize = numberOfResults * (sizeof(uint) + sizeof(float));
                byte[] buffer = new byte[bufferSize];
                Marshal.Copy(results, buffer, 0, (int)bufferSize);
                using (MemoryStream memoryStream = new MemoryStream(buffer))
                {
                    using (BinaryReader reader = new BinaryReader(memoryStream))
                    {
                        for (uint i = 0; i < numberOfResults; i++)
                        {
                            uint key = reader.ReadUInt32();
                            float distance = reader.ReadSingle();
                            keysAndDistances.Add(new KeyValuePair<uint, float>(key, distance));
                        }
                    }
                }

                // Free.
                NativeFreeKeysAndDistances(results);
            }
        }
    }
}
