using System;
using System.Collections.Generic;
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
		static int currBuff;
		static int capturedAt;
		static List<int> verticalLinesCH1;
		static List<int> verticalLinesCH2;
		static float phase;
		static float amplitude;
		static float Fin;
		static float Fout;
		static int triggerLevel;
		static Form1 mainForm;

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
		public static float getPhase() {
			return phase;
		}
		public static float getAmplitude() {
			return amplitude;
		}
		public static float getFin() {
			return Fin;
		}
		public static float getFout() {
			return Fout;
		}
		public static int getTriggerLevel() {
			return triggerLevel;
		}
		public static void setDACPeriod(float freq) {
			// _serialPort.Write();

			var byteArray = new byte[4];
			Buffer.BlockCopy(new float[]{freq}, 0, byteArray, 0, byteArray.Length);
			_serialPort.Write(new byte[] { (byte)'f', byteArray[0], byteArray[1], byteArray[2], byteArray[3] }, 0, 5);
		}

		public static void Read() {
			while (_continue) {
				try {
					string msg = _serialPort.ReadLine();
					Console.WriteLine("Recv: " + msg);
					if (msg.Equals("clr")) {
						dataPtr1 = 0;
						dataPtr2 = 0;

						currBuff = 1;

						verticalLinesCH1.Clear();
						verticalLinesCH2.Clear();
					} else if (msg.StartsWith("capat")) {
						capturedAt = int.Parse(msg.Split(' ')[1]);
					} else if (msg.StartsWith("triglv")) {
						triggerLevel = int.Parse(msg.Split(' ')[1]);
						mainForm.Invalidate();
					} else if (msg.StartsWith("v1")) { // vertical line channel 1
						verticalLinesCH1.Add(int.Parse(msg.Split(' ')[1]));
					} else if (msg.StartsWith("v2")) { // vertical line channel 2
						verticalLinesCH2.Add(int.Parse(msg.Split(' ')[1]));
					} else if (msg.StartsWith("phase")) {
						phase = int.Parse(msg.Split(' ')[1]) / 100.0f;
					} else if (msg.StartsWith("amp")) {
						amplitude = int.Parse(msg.Split(' ')[1]) / 100.0f;
					} else if (msg.StartsWith("Fin")) {
						Fin = int.Parse(msg.Split(' ')[1]) / 100.0f;
					} else if (msg.StartsWith("Fout")) {
						Fout = int.Parse(msg.Split(' ')[1]) / 100.0f;
					} else if (msg.Equals("rst")) {
						if (currBuff == 1) {
							currBuff = 2;
						} else {
							currBuff = 1;
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