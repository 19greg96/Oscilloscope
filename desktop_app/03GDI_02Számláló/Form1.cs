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
        public Form1()
        {
            InitializeComponent();
            // 01. Frissítés átméretezéskor
            this.SetStyle(ControlStyles.ResizeRedraw, true);
			this.SetStyle(ControlStyles.OptimizedDoubleBuffer, true);
			UpdateStyles();
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            StringFormat format = new StringFormat();
            // Igazítás (Near – balra, Center, Far - jobbra)
            format.Alignment = StringAlignment.Near;
            // Horizontális igazítás (Near – fent, Center, Far - lent)
            format.LineAlignment = StringAlignment.Center;

			
			int vDiv = 1;
			int hJump = 1;
			int maxValue = 4096;
			int bottomOffset = 200;
			int bot = this.ClientSize.Height - bottomOffset;
			int triggerLevel = Program.getTriggerLevel(); // rising
			Pen dashed_pen = new Pen(Color.Red, 1);
			dashed_pen.DashStyle = DashStyle.Dash;
			e.Graphics.DrawLine(dashed_pen, 0, bot - triggerLevel / vDiv, this.ClientSize.Width, bot - triggerLevel / vDiv);
			e.Graphics.DrawLine(Pens.Black, 0, bot, this.ClientSize.Width, bot);
			e.Graphics.DrawLine(Pens.Black, 0, bot - maxValue / vDiv, this.ClientSize.Width, bot - maxValue / vDiv);
			e.Graphics.DrawString("0", this.Font, Brushes.Black, 0, bot, format);
			e.Graphics.DrawString("4096", this.Font, Brushes.Black, 0, bot - maxValue / vDiv, format);

			/*
			GraphicsPath myPath = new GraphicsPath();
			int[] buff1 = Program.getBuffer1();
			int[] buff2 = Program.getBuffer2();
			for (int i = 0; i < (Program.getPtr() - 1) / 2; i++) {
				int valX = bot - buff1[i];
				int nextValX = bot - buff1[i + 1];
				int valY = bot - buff2[i];
				int nextValY = bot - buff2[i + 1];
				myPath.AddLine(valX, valY, nextValX, nextValY);
			}
			e.Graphics.DrawPath(Pens.Cyan, myPath);
			*/

			
			GraphicsPath myPath = new GraphicsPath();
			int[] buff = Program.getBuffer1();
			for (int i = 0; i < Program.getPtr() - 1; i ++) {
				int val = bot - (int)(buff[i] / 409.60f);
				int nextVal = bot - (int)(buff[i + 1] / 409.60f);
				myPath.AddLine(i / hJump, val, i / hJump + 1, nextVal);
			}
			e.Graphics.DrawPath(Pens.Cyan, myPath);

			myPath = new GraphicsPath();
			buff = Program.getBuffer2();
			for (int i = 0; i < Program.getPtr() - 1; i++) {
				int val = bot - (int)(buff[i] / 409.60f);
				int nextVal = bot - (int)(buff[i + 1] / 409.60f);
				myPath.AddLine(i / hJump, val, i / hJump + 1, nextVal);
			}
			e.Graphics.DrawPath(Pens.Black, myPath);


			int capAt = Program.getCapturedAt() / hJump;
			e.Graphics.DrawLine(new Pen(Color.Blue, 1), capAt, 0, capAt, this.ClientSize.Height);

			foreach (int pos in Program.getVerticalLinesCH1()) {
				int tp1 = pos / hJump;
				e.Graphics.DrawLine(new Pen(Color.Red, 1), tp1, 0, tp1, this.ClientSize.Height);
			}
			foreach (int pos in Program.getVerticalLinesCH2()) {
				int tp1 = pos / hJump;
				e.Graphics.DrawLine(new Pen(Color.Green, 1), tp1, 0, tp1, this.ClientSize.Height);
			}

			label2.Text = Program.getPhase().ToString() + "°";
			label3.Text = Program.getAmplitude().ToString() + "dB";
			label4.Text = Program.getFin().ToString() + "Hz";
			label5.Text = Program.getFout().ToString() + "Hz";
		}

		private void Form1_Load(object sender, EventArgs e) {

		}

		private void trackBar1_Scroll(object sender, EventArgs e) {
			float freq = (float)Math.Pow(10.0, (double)trackBar1.Value / 100.0 * 6.0);
			Program.setDACPeriod(freq);
			label1.Text = freq.ToString() + "Hz";
		}

		private void trackBar2_Scroll(object sender, EventArgs e) {
			Program.setTriggerLevel(trackBar2.Value);
		}
	}

    
    
}