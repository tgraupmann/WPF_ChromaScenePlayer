using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace WPF_ChromaScenePlayer
{
    class PlayerDLL
    {
#if X64
        const string DLL_NAME = "DLL_ChromaScenePlayer64";
#else
        const string DLL_NAME = "DLL_ChromaScenePlayer";
#endif

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int ApplicationStart();

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int ApplicationQuit();

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int PlayerChromaInit();

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int PlayerChromaUninit();

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern int PlayerLoadScene(IntPtr path);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int PlayerSelectScene(int sceneIndex);

        public static int LoadScene(string path)
        {
            string pathPath = path;
            IntPtr lpPath = GetIntPtr(pathPath);
            int result = PlayerLoadScene(lpPath);
            FreeIntPtr(lpPath);
            return result;
        }

        #region Helpers (handle path conversions)

        /// <summary>
        /// Helper to convert string to IntPtr
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        private static IntPtr GetIntPtr(string path)
        {
            if (string.IsNullOrEmpty(path))
            {
                return IntPtr.Zero;
            }
            FileInfo fi = new FileInfo(path);
            byte[] array = ASCIIEncoding.ASCII.GetBytes(fi.FullName + "\0");
            IntPtr lpData = Marshal.AllocHGlobal(array.Length);
            Marshal.Copy(array, 0, lpData, array.Length);
            return lpData;
        }

        /// <summary>
        /// Helper to recycle the IntPtr
        /// </summary>
        /// <param name="lpData"></param>
        private static void FreeIntPtr(IntPtr lpData)
        {
            if (lpData != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(lpData);
            }
        }

        #endregion
    }
}
