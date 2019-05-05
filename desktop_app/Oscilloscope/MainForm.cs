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
		private float vDiv = 400.0f; // data is received in Program.getRadix() (1000 corresponds to mV)
		private float hDiv = 1.0f;

		private StringFormat labelFormat;
		private Pen triggerLevelPen;
		private Pen triggerPointPen;
		private Pen divLinePen;
		private Pen ch1Pen;
		private Pen ch2Pen;
		private Pen ch1VlinesPen;
		private Pen ch2VlinesPen;

		public bool doAutoConnect { get; set; }

		public MainForm()
        {
            InitializeComponent();
			SerialPortEnumerator.RegisterUsbDeviceNotification(this.Handle);

			SetStyle(ControlStyles.ResizeRedraw, true);
			SetStyle(ControlStyles.OptimizedDoubleBuffer, true);
			SetStyle(ControlStyles.AllPaintingInWmPaint, true);
			UpdateStyles();

			labelFormat = new StringFormat();
			labelFormat.Alignment = StringAlignment.Near; // horizontal align (Near – left, Center, Far - right)
			labelFormat.LineAlignment = StringAlignment.Center; // vertical align (Near – top, Center, Far - bottom)

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
			Program.MainFormWndProc(ref m);
		}

		protected override void OnPaint(PaintEventArgs e)
        {
			if (doAutoConnect) {
				Program.autoConnectSerialPort();
				doAutoConnect = false;
			}
			if (Program.BufferSize == 0) { // TODO: this should be "if (buffer is not full)"
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

				int signalCenter = ClientSize.Height / 2;
				int triggerLevel = Program.TriggerLevel;
				if (triggerLevel > Int32.MinValue) {
					e.Graphics.DrawLine(triggerLevelPen, 0, signalCenter - triggerLevel / vDiv, ClientSize.Width, signalCenter - triggerLevel / vDiv);
				}

				float vDivInterval = 2.5f * Program.Radix;
				if (vDiv < 400.0f) {
					vDivInterval = 0.5f * Program.Radix;
				}

				for (float i = -5.0f * Program.Radix; i < 5.0f * Program.Radix + 1; i += vDivInterval) {
					e.Graphics.DrawLine(divLinePen, 0, signalCenter + i / vDiv, ClientSize.Width, signalCenter + i / vDiv);
					e.Graphics.DrawString(String.Format("{0}V", -i / Program.Radix), Font, Brushes.Black, 0, signalCenter + i / vDiv, labelFormat);
				}
				if (Program.SamplingFrequency > 0) {
					float sampleTime_s = 1.0f / Program.SamplingFrequency;
					float hDivNum = 10.0f;
					float hDivInterval = ClientSize.Width / hDivNum;
					float hDivTime_s = hDivInterval * sampleTime_s * hDiv;
					for (int i = 0; i < hDivNum; i++) {
						e.Graphics.DrawLine(divLinePen, i * hDivInterval, 0, i * hDivInterval, ClientSize.Height);
						e.Graphics.DrawString(String.Format("{0}s", FormatExtensions.ToEngineeringNotation(hDivTime_s * i)), Font, Brushes.Black, i * hDivInterval, ClientSize.Height - 50, labelFormat);
					}
				}


				GraphicsPath myPath = new GraphicsPath();
				for (int i = 0; i < Program.BufferSize - 1; i++) {
					int val = signalCenter - (int)(Program.Buffer1[i + (int)(hScroll.Value * hDiv)] / vDiv);
					int nextVal = signalCenter - (int)(Program.Buffer1[i + (int)(hScroll.Value * hDiv) + 1] / vDiv);
					myPath.AddLine(i / hDiv, val, i / hDiv + 1, nextVal);
				}
				e.Graphics.DrawPath(ch1Pen, myPath);

				myPath = new GraphicsPath();
				for (int i = 0; i < Program.BufferSize - 1; i++) {
					int val = signalCenter - (int)(Program.Buffer2[i + (int)(hScroll.Value * hDiv)] / vDiv);
					int nextVal = signalCenter - (int)(Program.Buffer2[i + (int)(hScroll.Value * hDiv) + 1] / vDiv);
					myPath.AddLine(i / hDiv, val, i / hDiv + 1, nextVal);
				}
				e.Graphics.DrawPath(ch2Pen, myPath);


				int capAt = (int)(Program.CapturedAt / hDiv - hScroll.Value);
				e.Graphics.DrawLine(triggerPointPen, capAt, 0, capAt, ClientSize.Height);

				foreach (int pos in Program.VerticalLines1) {
					int tp1 = (int)(pos / hDiv - hScroll.Value);
					e.Graphics.DrawLine(ch1VlinesPen, tp1, 0, tp1, ClientSize.Height);
				}
				foreach (int pos in Program.VerticalLines2) {
					int tp1 = (int)(pos / hDiv - hScroll.Value);
					e.Graphics.DrawLine(ch2VlinesPen, tp1, 0, tp1, ClientSize.Height);
				}



				hScroll.Minimum = 0;
				int invisibleTicks = (int)(Program.BufferSize / hDiv) - ClientSize.Width;
				if (invisibleTicks > 0) {
					hScroll.Maximum = invisibleTicks + 511; // +hScroll.LargeChange-1;  Microsoft please..
					hScroll.LargeChange = 512;
					hScroll.Enabled = true;
				} else {
					hScroll.Value = 0;
					hScroll.Enabled = false;
				}
			}

			

			if (Program.ScopeDisplay != null) {
				e.Graphics.DrawImage(Program.ScopeDisplay, ClientSize.Width - Program.ScopeDisplay.Width - 50, 10);
				copyScreenButton.Enabled = true;
			} else {
				copyScreenButton.Enabled = false;
			}

			if (Program.IsSerialPortConnected == false) {
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
			Clipboard.SetImage(Program.ScopeDisplay);
		}

		private void hScroll_Scroll(object sender, ScrollEventArgs e) {
			Invalidate();
		}

		private void infoButton_Click(object sender, EventArgs e) {
			Program.openInfo();
		}

		private void settingsButton_Click(object sender, EventArgs e) {
			Program.openSettings();
		}
	}

    
    
}