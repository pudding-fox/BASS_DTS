using System.IO;
using System.Runtime.InteropServices;

namespace ManagedBass.Dts
{
    public static class BassDts
    {
        const string DllName = "bass_dts";

        public const ChannelType ChannelType = (ChannelType)0x1f200;

        public static int Module = 0;

        public static bool Load(string folderName = null)
        {
            if (Module == 0)
            {
                var fileName = default(string);
                if (!string.IsNullOrEmpty(folderName))
                {
                    fileName = Path.Combine(folderName, DllName);
                }
                else
                {
                    fileName = Path.Combine(Loader.FolderName, DllName);
                }
                Module = Bass.PluginLoad(string.Format("{0}.{1}", fileName, Loader.Extension));
            }
            return Module != 0;
        }

        public static bool Unload()
        {
            if (Module != 0)
            {
                if (!Bass.PluginFree(Module))
                {
                    return false;
                }
                Module = 0;
            }
            return true;
        }

        [DllImport(DllName)]
        static extern int BASS_DTS_StreamCreateFile(bool Memory, string File, long Offset, long Length, BassFlags Flags);

        public static int CreateStream(string File, long Offset = 0, long Length = 0, BassFlags Flags = BassFlags.Default)
        {
            return BASS_DTS_StreamCreateFile(false, File, Offset, Length, Flags);
        }
    }
}
