namespace Oscilloscope
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
			this.hScroll = new System.Windows.Forms.HScrollBar();
			this.settingsButton = new System.Windows.Forms.Button();
			this.infoButton = new System.Windows.Forms.Button();
			this.copyScreenButton = new System.Windows.Forms.Button();
			this.copyBufferButton = new System.Windows.Forms.Button();
			this.vDivZoomOutButton = new System.Windows.Forms.Button();
			this.vDivZoomInButton = new System.Windows.Forms.Button();
			this.hDivZoomOutButton = new System.Windows.Forms.Button();
			this.hDivZoomInButton = new System.Windows.Forms.Button();
			this.captureScreenButton = new System.Windows.Forms.Button();
			this.captureBufferButton = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// hScroll
			// 
			this.hScroll.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.hScroll.LargeChange = 50;
			this.hScroll.Location = new System.Drawing.Point(0, 456);
			this.hScroll.Name = "hScroll";
			this.hScroll.Size = new System.Drawing.Size(1084, 17);
			this.hScroll.TabIndex = 15;
			this.hScroll.Scroll += new System.Windows.Forms.ScrollEventHandler(this.hScroll_Scroll);
			// 
			// settingsButton
			// 
			this.settingsButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.settingsButton.BackgroundImage = global::Oscilloscope.Properties.Resources.wrench;
			this.settingsButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
			this.settingsButton.Location = new System.Drawing.Point(1046, 44);
			this.settingsButton.Name = "settingsButton";
			this.settingsButton.Size = new System.Drawing.Size(26, 26);
			this.settingsButton.TabIndex = 17;
			this.settingsButton.UseVisualStyleBackColor = true;
			this.settingsButton.Click += new System.EventHandler(this.settingsButton_Click);
			// 
			// infoButton
			// 
			this.infoButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.infoButton.BackgroundImage = global::Oscilloscope.Properties.Resources.information;
			this.infoButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
			this.infoButton.Location = new System.Drawing.Point(1046, 12);
			this.infoButton.Name = "infoButton";
			this.infoButton.Size = new System.Drawing.Size(26, 26);
			this.infoButton.TabIndex = 16;
			this.infoButton.UseVisualStyleBackColor = true;
			this.infoButton.Click += new System.EventHandler(this.infoButton_Click);
			// 
			// copyScreenButton
			// 
			this.copyScreenButton.Enabled = false;
			this.copyScreenButton.Image = global::Oscilloscope.Properties.Resources.pictures;
			this.copyScreenButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.copyScreenButton.Location = new System.Drawing.Point(155, 44);
			this.copyScreenButton.Name = "copyScreenButton";
			this.copyScreenButton.Size = new System.Drawing.Size(105, 26);
			this.copyScreenButton.TabIndex = 14;
			this.copyScreenButton.Text = "Copy Screen";
			this.copyScreenButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.copyScreenButton.UseVisualStyleBackColor = true;
			this.copyScreenButton.Click += new System.EventHandler(this.copyScreenButton_Click);
			// 
			// copyBufferButton
			// 
			this.copyBufferButton.Enabled = false;
			this.copyBufferButton.Image = global::Oscilloscope.Properties.Resources.page_copy;
			this.copyBufferButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.copyBufferButton.Location = new System.Drawing.Point(44, 44);
			this.copyBufferButton.Name = "copyBufferButton";
			this.copyBufferButton.Size = new System.Drawing.Size(105, 26);
			this.copyBufferButton.TabIndex = 13;
			this.copyBufferButton.Text = "Copy Buffer";
			this.copyBufferButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.copyBufferButton.UseVisualStyleBackColor = true;
			this.copyBufferButton.Click += new System.EventHandler(this.copyBufferButton_Click);
			// 
			// vDivZoomOutButton
			// 
			this.vDivZoomOutButton.BackgroundImage = global::Oscilloscope.Properties.Resources.magifier_zoom_out;
			this.vDivZoomOutButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
			this.vDivZoomOutButton.Location = new System.Drawing.Point(12, 44);
			this.vDivZoomOutButton.Name = "vDivZoomOutButton";
			this.vDivZoomOutButton.Size = new System.Drawing.Size(26, 26);
			this.vDivZoomOutButton.TabIndex = 12;
			this.vDivZoomOutButton.UseVisualStyleBackColor = true;
			this.vDivZoomOutButton.Click += new System.EventHandler(this.vDivZoomOutButton_Click);
			// 
			// vDivZoomInButton
			// 
			this.vDivZoomInButton.BackgroundImage = global::Oscilloscope.Properties.Resources.magnifier_zoom_in;
			this.vDivZoomInButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
			this.vDivZoomInButton.Location = new System.Drawing.Point(12, 12);
			this.vDivZoomInButton.Name = "vDivZoomInButton";
			this.vDivZoomInButton.Size = new System.Drawing.Size(26, 26);
			this.vDivZoomInButton.TabIndex = 11;
			this.vDivZoomInButton.UseVisualStyleBackColor = true;
			this.vDivZoomInButton.Click += new System.EventHandler(this.vDivZoomInButton_Click);
			// 
			// hDivZoomOutButton
			// 
			this.hDivZoomOutButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.hDivZoomOutButton.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("hDivZoomOutButton.BackgroundImage")));
			this.hDivZoomOutButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
			this.hDivZoomOutButton.Location = new System.Drawing.Point(44, 427);
			this.hDivZoomOutButton.Name = "hDivZoomOutButton";
			this.hDivZoomOutButton.Size = new System.Drawing.Size(26, 26);
			this.hDivZoomOutButton.TabIndex = 10;
			this.hDivZoomOutButton.UseVisualStyleBackColor = true;
			this.hDivZoomOutButton.Click += new System.EventHandler(this.hDivZoomOutButton_Click);
			// 
			// hDivZoomInButton
			// 
			this.hDivZoomInButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.hDivZoomInButton.BackgroundImage = global::Oscilloscope.Properties.Resources.magnifier_zoom_in;
			this.hDivZoomInButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
			this.hDivZoomInButton.Location = new System.Drawing.Point(12, 427);
			this.hDivZoomInButton.Name = "hDivZoomInButton";
			this.hDivZoomInButton.Size = new System.Drawing.Size(26, 26);
			this.hDivZoomInButton.TabIndex = 9;
			this.hDivZoomInButton.UseVisualStyleBackColor = true;
			this.hDivZoomInButton.Click += new System.EventHandler(this.hDivZoomInButton_Click);
			// 
			// captureScreenButton
			// 
			this.captureScreenButton.Image = global::Oscilloscope.Properties.Resources.picture;
			this.captureScreenButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.captureScreenButton.Location = new System.Drawing.Point(155, 12);
			this.captureScreenButton.Name = "captureScreenButton";
			this.captureScreenButton.Size = new System.Drawing.Size(105, 26);
			this.captureScreenButton.TabIndex = 8;
			this.captureScreenButton.Text = "Capture Screen";
			this.captureScreenButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.captureScreenButton.UseVisualStyleBackColor = true;
			this.captureScreenButton.Click += new System.EventHandler(this.captureScreenBtn_Click);
			// 
			// captureBufferButton
			// 
			this.captureBufferButton.Image = global::Oscilloscope.Properties.Resources.page;
			this.captureBufferButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.captureBufferButton.Location = new System.Drawing.Point(44, 12);
			this.captureBufferButton.Name = "captureBufferButton";
			this.captureBufferButton.Size = new System.Drawing.Size(105, 26);
			this.captureBufferButton.TabIndex = 7;
			this.captureBufferButton.Text = "Capture Buffer";
			this.captureBufferButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.captureBufferButton.UseVisualStyleBackColor = true;
			this.captureBufferButton.Click += new System.EventHandler(this.captureBufferBtn_Click);
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(1084, 473);
			this.Controls.Add(this.settingsButton);
			this.Controls.Add(this.infoButton);
			this.Controls.Add(this.hScroll);
			this.Controls.Add(this.copyScreenButton);
			this.Controls.Add(this.copyBufferButton);
			this.Controls.Add(this.vDivZoomOutButton);
			this.Controls.Add(this.vDivZoomInButton);
			this.Controls.Add(this.hDivZoomOutButton);
			this.Controls.Add(this.hDivZoomInButton);
			this.Controls.Add(this.captureScreenButton);
			this.Controls.Add(this.captureBufferButton);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MinimumSize = new System.Drawing.Size(316, 163);
			this.Name = "MainForm";
			this.Text = "Form1";
			this.ResumeLayout(false);

        }

		#endregion
		private System.Windows.Forms.Button captureBufferButton;
		private System.Windows.Forms.Button captureScreenButton;
		private System.Windows.Forms.Button hDivZoomInButton;
		private System.Windows.Forms.Button hDivZoomOutButton;
		private System.Windows.Forms.Button vDivZoomOutButton;
		private System.Windows.Forms.Button vDivZoomInButton;
		private System.Windows.Forms.Button copyScreenButton;
		private System.Windows.Forms.Button copyBufferButton;
		private System.Windows.Forms.HScrollBar hScroll;
		private System.Windows.Forms.Button infoButton;
		private System.Windows.Forms.Button settingsButton;
	}
}

