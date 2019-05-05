using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO.Ports;
using System.Management;
using System.Threading;
using System.Windows.Forms;

namespace Oscilloscope
{
    static class Program
    {
		static SerialPort _serialPort;
		public static bool serialPortConnected = false;
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
		static int triggerLevel = Int32.MinValue;
		static MainForm mainForm;
		static SettingsForm settingsForm;
		private static int screenWidth;
		private static int screenHeight;
		private static int screenNumBytes;
		private static float samplingFrequency = -1.0f;
		private static float radix = 1.0f;
		private static int numberOfBufferCopies = 0;
		public static bool serialPortsListUpdateNeeded = true;
		private static List<SerialPortEnumerator.PortInfo> serialPortsList;
		private static Thread readThread;
		public static string selectedSerialPort;

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

			autoConnectSerialPort();

			mainForm = new MainForm();
			settingsForm = new SettingsForm();
			Application.Run(mainForm);
			endSerialPortReadThread();
		}
		public static void autoConnectSerialPort() {
			if (serialPortConnected == false) {
				selectedSerialPort = "";
				foreach (SerialPortEnumerator.PortInfo pi in Program.getPortList()) {
					if (pi.Description.Contains("STLink")) {
						selectedSerialPort = pi.Name;
					}
				}
				if (selectedSerialPort != "") {
					startSerialPortReadThread(selectedSerialPort);
				}
			}
		}

		public static void startSerialPortReadThread(string portName) {
			endSerialPortReadThread();

			readThread = new Thread(Read);
			isScreenCapture = false;
			_serialPort = new SerialPort();

			_serialPort.PortName = portName; // "COM11"
			_serialPort.BaudRate = 115200;
			_serialPort.Parity = Parity.None;
			_serialPort.DataBits = 8;
			_serialPort.StopBits = StopBits.One;
			_serialPort.Handshake = Handshake.None;

			_serialPort.ReadTimeout = 1000;
			_serialPort.WriteTimeout = 500;

			try {
				_serialPort.Open();
			} catch (System.IO.IOException) {
				// com port does not exist
				return;
			} catch (System.UnauthorizedAccessException) {
				// com port is in use
				return;
			}

			serialPortConnected = true;
			readThread.Start();
			mainForm.Invalidate();
		}
		private static void endSerialPortReadThread() {
			tryCloseSerialPort();

			if (readThread != null) {
				if (readThread.ThreadState != ThreadState.Unstarted) {
					serialPortConnected = false;
					readThread.Join();
				}
			}
			mainForm.Invalidate();
		}
		public static List<SerialPortEnumerator.PortInfo> getPortList() {
			if (serialPortsListUpdateNeeded) {
				serialPortsList = SerialPortEnumerator.FindComPorts();
				serialPortsListUpdateNeeded = false; // TODO: not thread safe
			}
			return serialPortsList;
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
		public static int getBufferSize() {
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

		public static void copyBuffer() {
			numberOfBufferCopies++;

			int[] buff = getBuffer1();
			string str = String.Format("ch1_{0} = [", numberOfBufferCopies);
			for (int i = 0; i < getBufferSize(); i++) {
				str += (buff[i] / getRadix()).ToString() + " ";
			}
			str += String.Format("];\nch2_{0} = [", numberOfBufferCopies);

			buff = getBuffer2();
			for (int i = 0; i < getBufferSize(); i++) {
				str += (buff[i] / getRadix()).ToString() + " ";
			}
			str += "];\n";
			if (getSamplingFrequency() > 0) {
				str += String.Format("Fs = {0};\n", getSamplingFrequency());
			}
			if (getBufferSize() > 0) {
				str += String.Format("N = {0};\n", getBufferSize()); // number of samples
			}
			str += String.Format("Y = fft(ch1_{0}); f = Fs / 2 * linspace(0, 1, N / 2 + 1); plot(f, 20 * log10(abs(Y(1:N / 2 + 1)))); xlabel('f (Hz)');", numberOfBufferCopies);

			Clipboard.SetText(str);
		}

		public static void Read() {
			while (serialPortConnected) {
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
				} catch (System.IO.IOException) {
					serialPortConnected = false;
				} catch (Exception) {}
			}
			tryCloseSerialPort();
		}

		private static void tryCloseSerialPort() {
			try {
				_serialPort.Close();
			} catch (Exception) { }
		}
		

		internal static void openSettings() {
			settingsForm.populateFields();
			if (settingsForm.ShowDialog() != DialogResult.Cancel) {
				selectedSerialPort = settingsForm.SelectedCOMPort;
				startSerialPortReadThread(settingsForm.SelectedCOMPort);
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