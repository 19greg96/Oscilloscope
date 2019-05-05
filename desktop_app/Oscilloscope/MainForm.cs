using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Text;
using System.Windows.Forms;

namespace Oscilloscope
{
    public partial class MainForm : Form
    {
		float vDiv = 400.0f; // data is received in Program.getRadix() (1000 corresponds to mV)
		float hDiv = 1.0f;

		private Pen triggerLevelPen;
		private Pen triggerPointPen;
		private Pen divLinePen;
		private Pen ch1Pen;
		private Pen ch2Pen;
		private Pen ch1VlinesPen;
		private Pen ch2VlinesPen;

		public MainForm()
        {
            InitializeComponent();
			SerialPortEnumerator.RegisterUsbDeviceNotification(this.Handle);

			SetStyle(ControlStyles.ResizeRedraw, true);
			SetStyle(ControlStyles.OptimizedDoubleBuffer, true);
			SetStyle(ControlStyles.AllPaintingInWmPaint, true);
			UpdateStyles();

			triggerLevelPen = new Pen(Color.RoyalBlue, 1);
			triggerLevelPen.DashStyle = DashStyle.Dash;
			triggerPointPen = new Pen(Color.DodgerBlue, 1);

			divLinePen = new Pen(Color.Gray, 1);
			divLinePen.DashStyle = DashStyle.Dash;

			ch1Pen = Pens.Red;
			ch2Pen = Pens.Green;

			ch1VlinesPen = Pens.Orange;
			ch2VlinesPen = Pens.GreenYellow;
		}

		protected override void WndProc(ref Message m) {
			base.WndProc(ref m);
			if (m.Msg == SerialPortEnumerator.WmDevicechange) {
				switch ((int)m.WParam) {
					case SerialPortEnumerator.DbtDeviceremovecomplete: // device removed
						Program.serialPortsListUpdateNeeded = true;
						Invalidate();
						break;
					case SerialPortEnumerator.DbtDevicearrival: // device added
						Program.serialPortsListUpdateNeeded = true;
						Invalidate();
						break;
				}
			}
		}

		protected override void OnPaint(PaintEventArgs e)
        {
			if (Program.getBufferSize() == 0) {
				copyBufferButton.Enabled = false;
				vDivZoomInButton.Enabled = false;
				vDivZoomOutButton.Enabled = false;
				hDivZoomInButton.Enabled = false;
				hDivZoomOutButton.Enabled = false;

				hScroll.Enabled = false;
			} else {
				copyBufferButton.Enabled = true;
				vDivZoomInButton.Enabled = true;
				vDivZoomOutButton.Enabled = true;
				hDivZoomInButton.Enabled = true;
				hDivZoomOutButton.Enabled = true;


				StringFormat format = new StringFormat();
				format.Alignment = StringAlignment.Near; // horizontal align (Near – left, Center, Far - right)
				format.LineAlignment = StringAlignment.Center; // vertical align (Near – top, Center, Far - bottom)

				int signalCenter = ClientSize.Height / 2;
				int triggerLevel = Program.getTriggerLevel();
				if (triggerLevel > Int32.MinValue) {
					e.Graphics.DrawLine(triggerLevelPen, 0, signalCenter - triggerLevel / vDiv, ClientSize.Width, signalCenter - triggerLevel / vDiv);
				}

				float vDivInterval = 2.5f * Program.getRadix();
				if (vDiv < 400.0f) {
					vDivInterval = 0.5f * Program.getRadix();
				}

				for (float i = -5.0f * Program.getRadix(); i < 5.0f * Program.getRadix() + 1; i += vDivInterval) {
					e.Graphics.DrawLine(divLinePen, 0, signalCenter + i / vDiv, ClientSize.Width, signalCenter + i / vDiv);
					e.Graphics.DrawString(String.Format("{0}V", -i / Program.getRadix()), Font, Brushes.Black, 0, signalCenter + i / vDiv, format);
				}
				if (Program.getSamplingFrequency() > 0) {
					float sampleTime_s = 1.0f / Program.getSamplingFrequency();
					float hDivNum = 10.0f;
					float hDivInterval = ClientSize.Width / hDivNum;
					float hDivTime_s = hDivInterval * sampleTime_s * hDiv;
					for (int i = 0; i < hDivNum; i++) {
						e.Graphics.DrawLine(divLinePen, i * hDivInterval, 0, i * hDivInterval, ClientSize.Height);
						e.Graphics.DrawString(String.Format("{0}s", FormatExtensions.ToEngineeringNotation(hDivTime_s * i)), Font, Brushes.Black, i * hDivInterval, ClientSize.Height - 50, format);
					}
				}


				GraphicsPath myPath = new GraphicsPath();
				int[] buff = Program.getBuffer1();
				for (int i = 0; i < Program.getBufferSize() - 1; i++) {
					int val = signalCenter - (int)(buff[i + (int)(hScroll.Value * hDiv)] / vDiv);
					int nextVal = signalCenter - (int)(buff[i + (int)(hScroll.Value * hDiv) + 1] / vDiv);
					myPath.AddLine(i / hDiv, val, i / hDiv + 1, nextVal);
				}
				e.Graphics.DrawPath(ch1Pen, myPath);

				myPath = new GraphicsPath();
				buff = Program.getBuffer2();
				for (int i = 0; i < Program.getBufferSize() - 1; i++) {
					int val = signalCenter - (int)(buff[i + (int)(hScroll.Value * hDiv)] / vDiv);
					int nextVal = signalCenter - (int)(buff[i + (int)(hScroll.Value * hDiv) + 1] / vDiv);
					myPath.AddLine(i / hDiv, val, i / hDiv + 1, nextVal);
				}
				e.Graphics.DrawPath(ch2Pen, myPath);


				int capAt = (int)(Program.getCapturedAt() / hDiv - hScroll.Value);
				e.Graphics.DrawLine(triggerPointPen, capAt, 0, capAt, ClientSize.Height);

				foreach (int pos in Program.getVerticalLinesCH1()) {
					int tp1 = (int)(pos / hDiv - hScroll.Value);
					e.Graphics.DrawLine(ch1VlinesPen, tp1, 0, tp1, ClientSize.Height);
				}
				foreach (int pos in Program.getVerticalLinesCH2()) {
					int tp1 = (int)(pos / hDiv - hScroll.Value);
					e.Graphics.DrawLine(ch2VlinesPen, tp1, 0, tp1, ClientSize.Height);
				}



				hScroll.Minimum = 0;
				int invisibleTicks = (int)(Program.getBufferSize() / hDiv) - ClientSize.Width;
				if (invisibleTicks > 0) {
					hScroll.Maximum = invisibleTicks + 511; // +hScroll.LargeChange-1;  Microsoft please..
					hScroll.LargeChange = 512;
					hScroll.Enabled = true;
				} else {
					hScroll.Value = 0;
					hScroll.Enabled = false;
				}
			}

			

			if (Program.getScreenCaptureBitmap() != null) {
				e.Graphics.DrawImage(Program.getScreenCaptureBitmap(), ClientSize.Width - Program.getScreenCaptureBitmap().Width - 50, 10);
				copyScreenButton.Enabled = true;
			} else {
				copyScreenButton.Enabled = false;
			}

			if (Program.serialPortConnected == false) {
				captureBufferButton.Enabled = false;
				captureScreenButton.Enabled = false;
			} else {
				captureBufferButton.Enabled = true;
				captureScreenButton.Enabled = true;
			}
		}

		private void captureBufferBtn_Click(object sender, EventArgs e) {
			Program.captureBuffer();
		}

		private void captureScreenBtn_Click(object sender, EventArgs e) {
			Program.captureScreen();
		}

		private void vDivZoomInButton_Click(object sender, EventArgs e) {
			vDiv /= 2.0f;
			Invalidate();
		}

		private void vDivZoomOutButton_Click(object sender, EventArgs e) {
			vDiv *= 2.0f;
			Invalidate();
		}

		private void hDivZoomInButton_Click(object sender, EventArgs e) {
			hDiv /= 2.0f;
			Invalidate();
		}

		private void hDivZoomOutButton_Click(object sender, EventArgs e) {
			hDiv *= 2.0f;
			Invalidate();
		}

		private void copyBufferButton_Click(object sender, EventArgs e) {
			Program.copyBuffer();
		}

		private void copyScreenButton_Click(object sender, EventArgs e) {
			Clipboard.SetImage(Program.getScreenCaptureBitmap());
		}

		private void hScroll_Scroll(object sender, ScrollEventArgs e) {
			Invalidate();
		}

		private void infoButton_Click(object sender, EventArgs e) {
			MessageBox.Show("Simple two channel oscilloscope, function generator and bode plotter for STM32 NUCLEO-F446RE board.\n\nIcons by Mark James. http://famfamfam.com", "About", MessageBoxButtons.OK, MessageBoxIcon.Information);
		}

		private void settingsButton_Click(object sender, EventArgs e) {
			Program.openSettings();
		}
	}

    
    
}