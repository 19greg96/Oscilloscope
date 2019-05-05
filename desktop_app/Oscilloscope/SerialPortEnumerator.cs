using System;
using System.Collections.Generic;
using System.Management;
using System.Runtime.InteropServices;
using System.Text;

namespace Oscilloscope {
	class SerialPortEnumerator { // https://stackoverflow.com/a/34058613/1018376
		// Class to contain the port info.
		public class PortInfo {
			public string Name;
			public string Description;
		}

		// Method to prepare the WMI query connection options.
		public static ConnectionOptions PrepareOptions() {
			ConnectionOptions options = new ConnectionOptions();
			options.Impersonation = ImpersonationLevel.Impersonate;
			options.Authentication = AuthenticationLevel.Default;
			options.EnablePrivileges = true;
			return options;
		}

		// Method to prepare WMI query management scope.
		public static ManagementScope PrepareScope(string machineName, ConnectionOptions options, string path) {
			ManagementScope scope = new ManagementScope();
			scope.Path = new ManagementPath(@"\\" + machineName + path);
			scope.Options = options;
			scope.Connect();
			return scope;
		}

		// Method to retrieve the list of all COM ports.
		public static List<PortInfo> FindComPorts() {
			List<PortInfo> portList = new List<PortInfo>();
			ConnectionOptions options = PrepareOptions();
			ManagementScope scope = PrepareScope(Environment.MachineName, options, @"\root\CIMV2");

			// Prepare the query and searcher objects.
			ObjectQuery objectQuery = new ObjectQuery("SELECT * FROM Win32_PnPEntity WHERE ConfigManagerErrorCode = 0");
			ManagementObjectSearcher portSearcher = new ManagementObjectSearcher(scope, objectQuery);

			using (portSearcher) {
				string caption = null;
				// Invoke the searcher and search through each management object for a COM port.
				foreach (ManagementObject currentObject in portSearcher.Get()) {
					if (currentObject != null) {
						object currentObjectCaption = currentObject["Caption"];
						if (currentObjectCaption != null) {
							caption = currentObjectCaption.ToString();
							if (caption.Contains("(COM")) {
								PortInfo portInfo = new PortInfo();
								portInfo.Name = caption.Substring(caption.LastIndexOf("(COM")).Replace("(", string.Empty).Replace(")", string.Empty);
								portInfo.Description = caption;
								portList.Add(portInfo);
								Console.WriteLine(portInfo.Name + " " + portInfo.Description);
							}
						}
					}
				}
			}
			return portList;
		}


		public const int DbtDevicearrival = 0x8000; // system detected a new device        
		public const int DbtDeviceremovecomplete = 0x8004; // device is gone      
		public const int WmDevicechange = 0x0219; // device change event      
		private const int DbtDevtypDeviceinterface = 5;
		private static readonly Guid GuidDevinterfaceUSBDevice = new Guid("A5DCBF10-6530-11D2-901F-00C04FB951ED"); // USB devices
		private static IntPtr notificationHandle;

		/// <summary>
		/// Registers a window to receive notifications when USB devices are plugged or unplugged.
		/// </summary>
		/// <param name="windowHandle">Handle to the window receiving notifications.</param>
		public static void RegisterUsbDeviceNotification(IntPtr windowHandle) {
			DevBroadcastDeviceinterface dbi = new DevBroadcastDeviceinterface {
				DeviceType = DbtDevtypDeviceinterface,
				Reserved = 0,
				ClassGuid = GuidDevinterfaceUSBDevice,
				Name = 0
			};

			dbi.Size = Marshal.SizeOf(dbi);
			IntPtr buffer = Marshal.AllocHGlobal(dbi.Size);
			Marshal.StructureToPtr(dbi, buffer, true);

			notificationHandle = RegisterDeviceNotification(windowHandle, buffer, 0);
		}

		/// <summary>
		/// Unregisters the window for USB device notifications
		/// </summary>
		public static void UnregisterUsbDeviceNotification() {
			UnregisterDeviceNotification(notificationHandle);
		}

		[DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
		private static extern IntPtr RegisterDeviceNotification(IntPtr recipient, IntPtr notificationFilter, int flags);

		[DllImport("user32.dll")]
		private static extern bool UnregisterDeviceNotification(IntPtr handle);

		[StructLayout(LayoutKind.Sequential)]
		private struct DevBroadcastDeviceinterface {
			internal int Size;
			internal int DeviceType;
			internal int Reserved;
			internal Guid ClassGuid;
			internal short Name;
		}
	}
}
