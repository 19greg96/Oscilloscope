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
		public static int BufferSize { get; private set; }
		private static int dataPtr1;
		private static int dataPtr2;
		public static int[] Buffer1 { get; private set; }
		public static int[] Buffer2 { get; private set; }
		private static int currBuff;

		public static Bitmap ScopeDisplay { get; private set; }
		private static int ScreenWidth { get; set; }
		private static int ScreenHeight { get; set; }
		private static int ScreenNumBytes { get; set; }
		private static int screenCurrByte;
		private static bool isScreenCapture; // screen capture vs buffer data
		
		
		public static int CapturedAt { get; private set; }
		public static List<int> VerticalLines1 { get; private set; }
		public static List<int> VerticalLines2 { get; private set; }
		public static int TriggerLevel { get; private set; } = Int32.MinValue;
		public static float SamplingFrequency { get; private set; } = -1.0f;
		public static float Radix { get; private set; } = 1.0f;
		
		private static SerialPort serialPort;
		public static bool IsSerialPortConnected { get; private set; } = false;
		private static bool SerialPortsListUpdateNeeded { get; set; } = true;
		public static string SelectedSerialPort { get; private set; }
		private static List<SerialPortEnumerator.PortInfo> serialPortsList;
		private static Thread readThread;

		private static int NumberOfBufferCopies { get; set; } = 0; // to postfix copied buffer variable names
		private static MainForm mainForm;
		private static SettingsForm settingsForm;


		public static List<SerialPortEnumerator.PortInfo> PortList {
			get {
				if (SerialPortsListUpdateNeeded) {
					serialPortsList = SerialPortEnumerator.FindComPorts();
					SerialPortsListUpdateNeeded = false; // TODO: not thread safe
				}
				return serialPortsList;
			}
		}



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
			Buffer1 = new int[16384]; // bad form, this should come from slave device
			dataPtr2 = 0;
			Buffer2 = new int[16384];

			VerticalLines1 = new List<int>();
			VerticalLines2 = new List<int>();

			mainForm = new MainForm();
			settingsForm = new SettingsForm();

			autoConnectSerialPort();

			Application.Run(mainForm);
			endSerialPortReadThread();
		}

		internal static void MainFormWndProc(ref Message m) {
			if (m.Msg == SerialPortEnumerator.WmDevicechange) {
				switch ((int)m.WParam) {
					case SerialPortEnumerator.DbtDeviceremovecomplete: // device removed
						SerialPortsListUpdateNeeded = true;
						mainForm.Invalidate();
						break;
					case SerialPortEnumerator.DbtDevicearrival: // device added
						SerialPortsListUpdateNeeded = true;
						mainForm.doAutoConnect = true;
						mainForm.Invalidate();
						break;
				}
			}
		}

		public static void autoConnectSerialPort() {
			if (IsSerialPortConnected == false) {
				SelectedSerialPort = "";
				foreach (SerialPortEnumerator.PortInfo pi in Program.PortList) {
					if (pi.Description.Contains("STLink")) {
						SelectedSerialPort = pi.Name;
					}
				}
				if (SelectedSerialPort != "") {
					startSerialPortReadThread(SelectedSerialPort);
				}
			}
		}

		public static void startSerialPortReadThread(string portName) {
			endSerialPortReadThread();

			readThread = new Thread(Read);
			isScreenCapture = false;
			serialPort = new SerialPort();

			serialPort.PortName = portName; // "COM11"
			serialPort.BaudRate = 115200;
			serialPort.Parity = Parity.None;
			serialPort.DataBits = 8;
			serialPort.StopBits = StopBits.One;
			serialPort.Handshake = Handshake.None;

			serialPort.ReadTimeout = 1000;
			serialPort.WriteTimeout = 500;

			try {
				serialPort.Open();
			} catch (System.IO.IOException) {
				// com port does not exist
				return;
			} catch (System.UnauthorizedAccessException) {
				// com port is in use
				return;
			}

			IsSerialPortConnected = true;
			readThread.Start();
			mainForm.Invalidate();
		}
		private static void endSerialPortReadThread() {
			tryCloseSerialPort();

			if (readThread != null) {
				if (readThread.ThreadState != ThreadState.Unstarted) {
					IsSerialPortConnected = false;
					readThread.Join();
				}
			}
			mainForm.Invalidate();
		}

		public static void copyBuffer() {
			NumberOfBufferCopies++;

			string str = String.Format("ch1_{0} = [", NumberOfBufferCopies);
			for (int i = 0; i < BufferSize; i++) {
				str += (Buffer1[i] / Radix).ToString() + " ";
			}
			str += String.Format("];\nch2_{0} = [", NumberOfBufferCopies);

			for (int i = 0; i < BufferSize; i++) {
				str += (Buffer2[i] / Radix).ToString() + " ";
			}
			str += "];\n";
			if (SamplingFrequency > 0) {
				str += String.Format("Fs = {0};\n", SamplingFrequency);
			}
			if (BufferSize > 0) {
				str += String.Format("N = {0};\n", BufferSize); // number of samples
			}
			str += String.Format("Y = fft(ch1_{0}); f = Fs / 2 * linspace(0, 1, N / 2 + 1); plot(f, 20 * log10(abs(Y(1:N / 2 + 1)))); xlabel('f (Hz)');", NumberOfBufferCopies);

			Clipboard.SetText(str);
		}

		public static void Read() {
			while (IsSerialPortConnected) {
				try { // TODO: communication protocol is lacking
					string msg = serialPort.ReadLine();
					// Console.WriteLine("Recv: " + msg);
					if (msg.Equals("clr")) {
						dataPtr1 = 0;
						dataPtr2 = 0;

						currBuff = 1;

						VerticalLines1.Clear();
						VerticalLines2.Clear();
					} else if (msg.StartsWith("s")) {
						ScreenWidth = int.Parse(msg.Split(' ')[1]);
						ScreenHeight = int.Parse(msg.Split(' ')[2]);
						ScreenNumBytes = int.Parse(msg.Split(' ')[3]);
						isScreenCapture = true;
						screenCurrByte = 0;

						ScopeDisplay = new Bitmap(ScreenWidth, ScreenHeight);
						// dataSize = int.Parse(msg.Split(' ')[1]);
					} else if (msg.StartsWith("capat")) {
						CapturedAt = int.Parse(msg.Split(' ')[1]);
					} else if (msg.StartsWith("fs")) {
						SamplingFrequency = float.Parse(msg.Split(' ')[1]);
					} else if (msg.StartsWith("radix")) { // 1000 means we receive values in mV
						Radix = (float)int.Parse(msg.Split(' ')[1]);
					} else if (msg.StartsWith("triglv")) {
						TriggerLevel = int.Parse(msg.Split(' ')[1]);
						mainForm.Invalidate();
					} else if (msg.StartsWith("v1")) { // vertical line channel 1
						VerticalLines1.Add(int.Parse(msg.Split(' ')[1]));
					} else if (msg.StartsWith("v2")) { // vertical line channel 2
						VerticalLines2.Add(int.Parse(msg.Split(' ')[1]));
					} else if (msg.Equals("rst")) {
						if (currBuff == 1) {
							currBuff = 2;
						} else {
							currBuff = 1;
						}
					} else {
						if (isScreenCapture) {
							int vertical8Pixels = int.Parse(msg);
							int x = screenCurrByte % ScreenWidth;
							int y = (screenCurrByte / ScreenWidth) * 8;

							if ((vertical8Pixels & 0x80) != 0) { ScopeDisplay.SetPixel(x, y + 7, Color.Black); } else { ScopeDisplay.SetPixel(x, y + 7, Color.White); }
							if ((vertical8Pixels & 0x40) != 0) { ScopeDisplay.SetPixel(x, y + 6, Color.Black); } else { ScopeDisplay.SetPixel(x, y + 6, Color.White); }
							if ((vertical8Pixels & 0x20) != 0) { ScopeDisplay.SetPixel(x, y + 5, Color.Black); } else { ScopeDisplay.SetPixel(x, y + 5, Color.White); }
							if ((vertical8Pixels & 0x10) != 0) { ScopeDisplay.SetPixel(x, y + 4, Color.Black); } else { ScopeDisplay.SetPixel(x, y + 4, Color.White); }

							if ((vertical8Pixels & 0x08) != 0) { ScopeDisplay.SetPixel(x, y + 3, Color.Black); } else { ScopeDisplay.SetPixel(x, y + 3, Color.White); }
							if ((vertical8Pixels & 0x04) != 0) { ScopeDisplay.SetPixel(x, y + 2, Color.Black); } else { ScopeDisplay.SetPixel(x, y + 2, Color.White); }
							if ((vertical8Pixels & 0x02) != 0) { ScopeDisplay.SetPixel(x, y + 1, Color.Black); } else { ScopeDisplay.SetPixel(x, y + 1, Color.White); }
							if ((vertical8Pixels & 0x01) != 0) { ScopeDisplay.SetPixel(x, y + 0, Color.Black); } else { ScopeDisplay.SetPixel(x, y + 0, Color.White); }

							screenCurrByte++;
							if (screenCurrByte == ScreenNumBytes) {
								isScreenCapture = false;
								mainForm.Invalidate();
							}
						} else {
							if (currBuff == 1) {
								Buffer1[dataPtr1] = int.Parse(msg);
								dataPtr1++;
								if (dataPtr1 > BufferSize) {
									BufferSize = dataPtr1;
								}
							} else {
								Buffer2[dataPtr2] = int.Parse(msg);
								dataPtr2++;
							}
						}
					}
				} catch (System.IO.IOException) {
					IsSerialPortConnected = false;
				} catch (Exception) {}
			}
			tryCloseSerialPort();
		}

		

		private static void tryCloseSerialPort() {
			try {
				serialPort.Close();
			} catch (Exception) { }
		}


		internal static void openInfo() {
			MessageBox.Show("Simple two channel oscilloscope, function generator and bode plotter for STM32 NUCLEO-F446RE board.\n\nIcons by Mark James. http://famfamfam.com", "About", MessageBoxButtons.OK, MessageBoxIcon.Information);
		}

		internal static void openSettings() {
			settingsForm.populateFields();
			if (settingsForm.ShowDialog() != DialogResult.Cancel) {
				SelectedSerialPort = settingsForm.SelectedCOMPort;
				startSerialPortReadThread(settingsForm.SelectedCOMPort);
			}
		}

		internal static void captureScreen() {
			serialPort.Write("s");
		}

		internal static void captureBuffer() {
			serialPort.Write("b");
		}

		internal static void setTriggerLevel(int value) {
			byte val = (byte)value;
			serialPort.Write(new byte[] { (byte)'t', val}, 0, 2);
		}
	}
}