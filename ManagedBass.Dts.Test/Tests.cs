using NUnit.Framework;
using System;
using System.Diagnostics;
using System.IO;
using System.Threading;

namespace ManagedBass.Dts.Test
{
    [TestFixture(BassFlags.Default, @"..\..\Media\01 In Chains.dts", 412.661)]
    [TestFixture(BassFlags.Default, @"..\..\Media\01 All Night Long.dts", 167.027)]
    [TestFixture(BassFlags.Default, @"..\..\Media\01 World In My Eyes.dts", 266.613)]
    [TestFixture(BassFlags.Default | BassFlags.Float, @"..\..\Media\01 In Chains.dts", 412.661)]
    [TestFixture(BassFlags.Default | BassFlags.Float, @"..\..\Media\01 All Night Long.dts", 167.027)]
    [TestFixture(BassFlags.Default | BassFlags.Float, @"..\..\Media\01 World In My Eyes.dts", 266.613)]
    public class Tests
    {
        private static readonly string CurrentDirectory = Path.GetDirectoryName(typeof(Tests).Assembly.Location);

        public Tests(BassFlags bassFlags, string fileName, double length)
        {
            this.BassFlags = bassFlags;
            this.FileName = fileName;
            this.Length = length;
        }

        public BassFlags BassFlags { get; private set; }

        public string FileName { get; private set; }

        public double Length { get; private set; }

        [SetUp]
        public void SetUp()
        {
            Assert.IsTrue(Loader.Load("bass"));
            Assert.IsTrue(BassDts.Load());
            Assert.IsTrue(Bass.Init(Bass.DefaultDevice));
        }

        [TearDown]
        public void TearDown()
        {
            BassDts.Unload();
            Bass.Free();
        }
        /// <summary>
        /// A basic end to end test.
        /// </summary>
        [Test]
        public void Test001()
        {
            var sourceChannel = BassDts.CreateStream(Path.Combine(CurrentDirectory, this.FileName), 0, 0, this.BassFlags);
            if (sourceChannel == 0)
            {
                Assert.Fail(string.Format("Failed to create source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var channelInfo = default(ChannelInfo);
            if (!Bass.ChannelGetInfo(sourceChannel, out channelInfo))
            {
                Assert.Fail(string.Format("Failed to get stream info: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            Assert.AreEqual(BassDts.ChannelType, channelInfo.ChannelType);

            if (!Bass.ChannelPlay(sourceChannel))
            {
                Assert.Fail(string.Format("Failed to play the playback stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var channelLength = Bass.ChannelGetLength(sourceChannel);
            var channelLengthSeconds = Bass.ChannelBytes2Seconds(sourceChannel, channelLength);

            do
            {
                if (Bass.ChannelIsActive(sourceChannel) == PlaybackState.Stopped)
                {
                    break;
                }

                var channelPosition = Bass.ChannelGetPosition(sourceChannel);
                var channelPositionSeconds = Bass.ChannelBytes2Seconds(sourceChannel, channelPosition);

                Debug.WriteLine(
                    "{0}/{1}",
                    TimeSpan.FromSeconds(channelPositionSeconds).ToString("g"),
                    TimeSpan.FromSeconds(channelLengthSeconds).ToString("g")
                );

                Thread.Sleep(1000);
            } while (true);

            if (!Bass.StreamFree(sourceChannel))
            {
                Assert.Fail(string.Format("Failed to free the source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }
        }

        /// <summary>
        /// Check seeking.
        /// </summary>
        [Test]
        public void Test002_1()
        {
            var sourceChannel = BassDts.CreateStream(Path.Combine(CurrentDirectory, this.FileName), 0, 0, this.BassFlags);
            if (sourceChannel == 0)
            {
                Assert.Fail(string.Format("Failed to create source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var channelInfo = default(ChannelInfo);
            if (!Bass.ChannelGetInfo(sourceChannel, out channelInfo))
            {
                Assert.Fail(string.Format("Failed to get stream info: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            Assert.AreEqual(BassDts.ChannelType, channelInfo.ChannelType);

            if (!Bass.ChannelPlay(sourceChannel))
            {
                Assert.Fail(string.Format("Failed to play the playback stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var channelLength = Bass.ChannelGetLength(sourceChannel);
            var channelLengthSeconds = Bass.ChannelBytes2Seconds(sourceChannel, channelLength);

            Bass.ChannelSetPosition(sourceChannel, Bass.ChannelSeconds2Bytes(sourceChannel, channelLengthSeconds - 10), PositionFlags.Bytes);

            Thread.Sleep(2000);

            {
                var channelPosition = Bass.ChannelGetPosition(sourceChannel);
                var channelPositionSeconds = Bass.ChannelBytes2Seconds(sourceChannel, channelPosition);

                Assert.IsTrue(channelPositionSeconds >= channelLengthSeconds - 10);
            }

            Thread.Sleep(10000);

            {
                var channelPosition = Bass.ChannelGetPosition(sourceChannel);
                var channelPositionSeconds = Bass.ChannelBytes2Seconds(sourceChannel, channelPosition);

                Assert.AreEqual(Math.Floor(this.Length), Math.Floor(channelPositionSeconds));
            }

            if (!Bass.StreamFree(sourceChannel))
            {
                Assert.Fail(string.Format("Failed to free the source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }
        }

        /// <summary>
        /// Check seeking.
        /// </summary>
        [Test]
        public void Test002_2()
        {
            var sourceChannel = BassDts.CreateStream(Path.Combine(CurrentDirectory, this.FileName), 0, 0, this.BassFlags);
            if (sourceChannel == 0)
            {
                Assert.Fail(string.Format("Failed to create source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var channelInfo = default(ChannelInfo);
            if (!Bass.ChannelGetInfo(sourceChannel, out channelInfo))
            {
                Assert.Fail(string.Format("Failed to get stream info: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            Assert.AreEqual(BassDts.ChannelType, channelInfo.ChannelType);

            if (!Bass.ChannelPlay(sourceChannel))
            {
                Assert.Fail(string.Format("Failed to play the playback stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var channelLength = Bass.ChannelGetLength(sourceChannel);
            var channelLengthSeconds = Bass.ChannelBytes2Seconds(sourceChannel, channelLength);

            Bass.ChannelSetPosition(sourceChannel, channelLength, PositionFlags.Bytes);

            var channelPosition = Bass.ChannelGetPosition(sourceChannel);
            var channelPositionSeconds = Bass.ChannelBytes2Seconds(sourceChannel, channelPosition);

            Assert.AreEqual(Math.Floor(this.Length), Math.Floor(channelPositionSeconds));

            if (!Bass.StreamFree(sourceChannel))
            {
                Assert.Fail(string.Format("Failed to free the source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }
        }

        /// <summary>
        /// Check length.
        /// </summary>
        [Test]
        public void Test003()
        {
            var sourceChannel = BassDts.CreateStream(Path.Combine(CurrentDirectory, this.FileName), 0, 0, this.BassFlags);
            if (sourceChannel == 0)
            {
                Assert.Fail(string.Format("Failed to create source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var channelInfo = default(ChannelInfo);
            if (!Bass.ChannelGetInfo(sourceChannel, out channelInfo))
            {
                Assert.Fail(string.Format("Failed to get stream info: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            Assert.AreEqual(BassDts.ChannelType, channelInfo.ChannelType);

            var channelLength = Bass.ChannelGetLength(sourceChannel);
            var channelLengthSeconds = Bass.ChannelBytes2Seconds(sourceChannel, channelLength);

            Assert.AreEqual(Math.Floor(this.Length), Math.Floor(channelLengthSeconds));

            if (!Bass.StreamFree(sourceChannel))
            {
                Assert.Fail(string.Format("Failed to free the source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }
        }
    }
}