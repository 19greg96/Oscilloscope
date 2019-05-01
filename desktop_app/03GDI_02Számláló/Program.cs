using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO.Ports;
using System.Threading;
using System.Windows.Forms;

namespace Demo2
{
    static class Program
    {

		static SerialPort _serialPort;
		static bool _continue;
		static int dataPtr1;
		static int dataPtr2;
		static int bufferSize;
		static int[] data1;
		static int[] data2;
		static Bitmap scopeDisplay;
		static bool isScreenCapture;
		private static int screenCurrByte;
		static int currBuff;
		static int capturedAt;
		static List<int> verticalLinesCH1;
		static List<int> verticalLinesCH2;
		static int triggerLevel;
		static Form1 mainForm;
		private static int screenWidth;
		private static int screenHeight;
		private static int screenNumBytes;
		private static float samplingFrequency;
		private static float radix = 1.0f;

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

			currBuff = 1;
			dataPtr1 = 0;
			data1 = new int[16384];
			dataPtr2 = 0;
			data2 = new int[16384];

			verticalLinesCH1 = new List<int>();
			verticalLinesCH2 = new List<int>();

			Thread readThread = new Thread(Read);
			_continue = true;
			isScreenCapture = false;

			_serialPort = new SerialPort();
			// _serialPort.PortName = "COM5";
			_serialPort.PortName = "COM11";
			_serialPort.BaudRate = 115200;
			_serialPort.Parity = Parity.None;
			_serialPort.DataBits = 8;
			_serialPort.StopBits = StopBits.One;
			_serialPort.Handshake = Handshake.None;

			_serialPort.ReadTimeout = 10000;
			_serialPort.WriteTimeout = 500;

			_serialPort.Open();
			readThread.Start();

			mainForm = new Form1();
			Application.Run(mainForm);
			_continue = false;

			readThread.Join();
			_serialPort.Close();
		}

		public static float getRadix() {
			return radix;
		}
		public static float getSamplingFrequency() {
			return samplingFrequency;
		}

		public static int[] getBuffer1() {
			return data1;
		}
		public static int[] getBuffer2() {
			return data2;
		}
		public static int getPtr() {
			return bufferSize;
		}
		public static int getCapturedAt() {
			return capturedAt;
		}
		public static List<int> getVerticalLinesCH1() {
			return verticalLinesCH1;
		}
		public static List<int> getVerticalLinesCH2() {
			return verticalLinesCH2;
		}
		public static int getTriggerLevel() {
			return triggerLevel;
		}
		public static Bitmap getScreenCaptureBitmap() {
			return scopeDisplay;
		}

		public static void Read() {
			while (_continue) {
				try {
					string msg = _serialPort.ReadLine();
					// Console.WriteLine("Recv: " + msg);
					if (msg.Equals("clr")) {
						dataPtr1 = 0;
						dataPtr2 = 0;

						currBuff = 1;

						verticalLinesCH1.Clear();
						verticalLinesCH2.Clear();
					} else if (msg.StartsWith("s")) {
						screenWidth = int.Parse(msg.Split(' ')[1]);
						screenHeight = int.Parse(msg.Split(' ')[2]);
						screenNumBytes = int.Parse(msg.Split(' ')[3]);
						isScreenCapture = true;
						screenCurrByte = 0;

						scopeDisplay = new Bitmap(screenWidth, screenHeight);
						// dataSize = int.Parse(msg.Split(' ')[1]);
					} else if (msg.StartsWith("capat")) {
						capturedAt = int.Parse(msg.Split(' ')[1]);
					} else if (msg.StartsWith("fs")) {
						samplingFrequency = float.Parse(msg.Split(' ')[1]);
					} else if (msg.StartsWith("radix")) { // 1000 means we receive values in mV
						radix = (float)int.Parse(msg.Split(' ')[1]);
					} else if (msg.StartsWith("triglv")) {
						triggerLevel = int.Parse(msg.Split(' ')[1]);
						mainForm.Invalidate();
					} else if (msg.StartsWith("v1")) { // vertical line channel 1
						verticalLinesCH1.Add(int.Parse(msg.Split(' ')[1]));
					} else if (msg.StartsWith("v2")) { // vertical line channel 2
						verticalLinesCH2.Add(int.Parse(msg.Split(' ')[1]));
					} else if (msg.Equals("rst")) {
						if (currBuff == 1) {
							currBuff = 2;
						} else {
							currBuff = 1;
						}
					} else {
						if (isScreenCapture) {
							int vertical8Pixels = int.Parse(msg);
							int x = screenCurrByte % screenWidth;
							int y = (screenCurrByte / screenWidth) * 8;

							if ((vertical8Pixels & 0x80) != 0) { scopeDisplay.SetPixel(x, y + 7, Color.Black); } else { scopeDisplay.SetPixel(x, y + 7, Color.White); }
							if ((vertical8Pixels & 0x40) != 0) { scopeDisplay.SetPixel(x, y + 6, Color.Black); } else { scopeDisplay.SetPixel(x, y + 6, Color.White); }
							if ((vertical8Pixels & 0x20) != 0) { scopeDisplay.SetPixel(x, y + 5, Color.Black); } else { scopeDisplay.SetPixel(x, y + 5, Color.White); }
							if ((vertical8Pixels & 0x10) != 0) { scopeDisplay.SetPixel(x, y + 4, Color.Black); } else { scopeDisplay.SetPixel(x, y + 4, Color.White); }

							if ((vertical8Pixels & 0x08) != 0) { scopeDisplay.SetPixel(x, y + 3, Color.Black); } else { scopeDisplay.SetPixel(x, y + 3, Color.White); }
							if ((vertical8Pixels & 0x04) != 0) { scopeDisplay.SetPixel(x, y + 2, Color.Black); } else { scopeDisplay.SetPixel(x, y + 2, Color.White); }
							if ((vertical8Pixels & 0x02) != 0) { scopeDisplay.SetPixel(x, y + 1, Color.Black); } else { scopeDisplay.SetPixel(x, y + 1, Color.White); }
							if ((vertical8Pixels & 0x01) != 0) { scopeDisplay.SetPixel(x, y + 0, Color.Black); } else { scopeDisplay.SetPixel(x, y + 0, Color.White); }

							screenCurrByte++;
							if (screenCurrByte == screenNumBytes) {
								isScreenCapture = false;
								mainForm.Invalidate();
							}
						} else {
							if (currBuff == 1) {
								data1[dataPtr1] = int.Parse(msg);
								dataPtr1++;
								if (dataPtr1 > bufferSize) {
									bufferSize = dataPtr1;
								}
							} else {
								data2[dataPtr2] = int.Parse(msg);
								dataPtr2++;
							}
						}
					}
					
				} catch (Exception) {
					// Console.WriteLine("Timeout");
				}
				
			}
		}

		internal static void captureScreen() {
			_serialPort.Write("s");
		}

		internal static void captureBuffer() {
			_serialPort.Write("b");
		}

		internal static void setTriggerLevel(int value) {
			byte val = (byte)value;
			_serialPort.Write(new byte[] { (byte)'t', val}, 0, 2);
		}
	}
}