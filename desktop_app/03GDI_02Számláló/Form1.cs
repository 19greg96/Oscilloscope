using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Text;
using System.Windows.Forms;

namespace Demo2
{
    public partial class Form1 : Form
    {
		float vDiv = 400.0f; // data is received in Program.getRadix() (1000 corresponds to mV)
		float hDiv = 1.0f;

		private Pen triggerLevelPen;
		private Pen divLinePen;

		public Form1()
        {
            InitializeComponent();
            // 01. Frissítés átméretezéskor
            SetStyle(ControlStyles.ResizeRedraw, true);
			SetStyle(ControlStyles.OptimizedDoubleBuffer, true);
			SetStyle(ControlStyles.AllPaintingInWmPaint, true);
			UpdateStyles();

			triggerLevelPen = new Pen(Color.Red, 1);
			triggerLevelPen.DashStyle = DashStyle.Dash;
			divLinePen = new Pen(Color.Gray, 1);
			divLinePen.DashStyle = DashStyle.Dash;
		}

		protected override void OnPaint(PaintEventArgs e)
        {
            StringFormat format = new StringFormat();
            // Igazítás (Near – balra, Center, Far - jobbra)
            format.Alignment = StringAlignment.Near;
            // Horizontális igazítás (Near – fent, Center, Far - lent)
            format.LineAlignment = StringAlignment.Center;

			int bot = this.ClientSize.Height / 2;
			int triggerLevel = Program.getTriggerLevel();
			if (triggerLevel > Int32.MinValue) {
				e.Graphics.DrawLine(triggerLevelPen, 0, bot - triggerLevel / vDiv, this.ClientSize.Width, bot - triggerLevel / vDiv);
			}

			float vDivInterval = 2.5f * Program.getRadix();
			if (vDiv < 400.0f) {
				vDivInterval = 0.5f * Program.getRadix();
			}
			
			for (float i = -5.0f * Program.getRadix(); i < 5.0f * Program.getRadix() + 1; i += vDivInterval) {
				e.Graphics.DrawLine(divLinePen, 0, bot + i / vDiv, this.ClientSize.Width, bot + i / vDiv);
				e.Graphics.DrawString(String.Format("{0}V", -i / Program.getRadix()), this.Font, Brushes.Black, 0, bot + i / vDiv, format);
			}
			if (Program.getSamplingFrequency() > 0) {
				float sampleTime_s = 1.0f / Program.getSamplingFrequency();
				float hDivNum = 10.0f;
				float hDivInterval = this.ClientSize.Width / hDivNum;
				float hDivTime_s = hDivInterval * sampleTime_s * hDiv;
				for (int i = 0; i < hDivNum; i++) {
					e.Graphics.DrawLine(divLinePen, i * hDivInterval, 0, i * hDivInterval, this.ClientSize.Height);
					e.Graphics.DrawString(String.Format("{0}s", FormatExtensions.ToEngineeringNotation(hDivTime_s * i)), this.Font, Brushes.Black, i * hDivInterval, this.ClientSize.Height - 50, format);
				}
			}


			GraphicsPath myPath = new GraphicsPath();
			int[] buff = Program.getBuffer1();
			for (int i = 0; i < Program.getPtr() - 1; i ++) {
				int val = bot - (int)(buff[i + (int)(hScroll.Value * hDiv)] / vDiv);
				int nextVal = bot - (int)(buff[i + (int)(hScroll.Value * hDiv) + 1] / vDiv);
				myPath.AddLine(i / hDiv, val, i / hDiv + 1, nextVal);
			}
			e.Graphics.DrawPath(Pens.Cyan, myPath);

			myPath = new GraphicsPath();
			buff = Program.getBuffer2();
			for (int i = 0; i < Program.getPtr() - 1; i++) {
				int val = bot - (int)(buff[i + (int)(hScroll.Value * hDiv)] / vDiv);
				int nextVal = bot - (int)(buff[i + (int)(hScroll.Value * hDiv) + 1] / vDiv);
				myPath.AddLine(i / hDiv, val, i / hDiv + 1, nextVal);
			}
			e.Graphics.DrawPath(Pens.Black, myPath);


			int capAt = (int)(Program.getCapturedAt() / hDiv - hScroll.Value);
			e.Graphics.DrawLine(new Pen(Color.Blue, 1), capAt, 0, capAt, this.ClientSize.Height);

			foreach (int pos in Program.getVerticalLinesCH1()) {
				int tp1 = (int)(pos / hDiv - hScroll.Value);
				e.Graphics.DrawLine(new Pen(Color.Red, 1), tp1, 0, tp1, this.ClientSize.Height);
			}
			foreach (int pos in Program.getVerticalLinesCH2()) {
				int tp1 = (int)(pos / hDiv - hScroll.Value);
				e.Graphics.DrawLine(new Pen(Color.Green, 1), tp1, 0, tp1, this.ClientSize.Height);
			}

			if (Program.getScreenCaptureBitmap() != null) {
				e.Graphics.DrawImage(Program.getScreenCaptureBitmap(), 10, 10);
				copyScreenButton.Enabled = true;
			} else {
				copyScreenButton.Enabled = false;
			}
			if (Program.getPtr() == 0) {
				copyBufferButton.Enabled = false;
				hScroll.Enabled = false;
			} else {
				copyBufferButton.Enabled = true;
				
				int invisibleTicks = (int)(Program.getPtr() / hDiv) - this.ClientSize.Width;
				if (invisibleTicks > 0) {
					hScroll.Maximum = invisibleTicks + 511; // +hScroll.LargeChange-1;  Microsoft please..
					hScroll.LargeChange = 512;
					hScroll.Enabled = true;
				} else {
					hScroll.Enabled = false;
				}
				hScroll.Minimum = 0;
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
			int[] buff = Program.getBuffer1();
			string str = "ch1 = [";
			for (int i = 0; i < Program.getPtr(); i++) {
				str += (buff[i] / Program.getRadix()).ToString() + " ";
			}
			str += "];\nch2 = [";

			buff = Program.getBuffer2();
			for (int i = 0; i < Program.getPtr(); i++) {
				str += (buff[i] / Program.getRadix()).ToString() + " ";
			}
			str += "];\n";
			if (Program.getSamplingFrequency() > 0) {
				str += String.Format("Fs = {0};\n", Program.getSamplingFrequency());
			}
			if (Program.getPtr() > 0) {
				str += String.Format("N = {0};\n", Program.getPtr()); // number of samples
			}
			/*
			Y = fft(ch1); f = Fs/2*linspace(0,1,N/2+1); plot(f, 20*log10(abs(Y(1:N/2+1)))); xlabel('f (Hz)');
			*/

			Clipboard.SetText(str);
		}

		private void copyScreenButton_Click(object sender, EventArgs e) {
			Clipboard.SetImage(Program.getScreenCaptureBitmap());
		}

		private void hScroll_Scroll(object sender, ScrollEventArgs e) {
			Invalidate();
		}
	}

    
    
}