namespace Demo2
{
    partial class Form1
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
			this.captureBufferButton = new System.Windows.Forms.Button();
			this.captureScreenButton = new System.Windows.Forms.Button();
			this.hDivZoomInButton = new System.Windows.Forms.Button();
			this.hDivZoomOutButton = new System.Windows.Forms.Button();
			this.vDivZoomOutButton = new System.Windows.Forms.Button();
			this.vDivZoomInButton = new System.Windows.Forms.Button();
			this.copyScreenButton = new System.Windows.Forms.Button();
			this.copyBufferButton = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// captureBufferButton
			// 
			this.captureBufferButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.captureBufferButton.Location = new System.Drawing.Point(916, 12);
			this.captureBufferButton.Name = "captureBufferButton";
			this.captureBufferButton.Size = new System.Drawing.Size(75, 40);
			this.captureBufferButton.TabIndex = 7;
			this.captureBufferButton.Text = "Capture Buffer";
			this.captureBufferButton.UseVisualStyleBackColor = true;
			this.captureBufferButton.Click += new System.EventHandler(this.captureBufferBtn_Click);
			// 
			// captureScreenButton
			// 
			this.captureScreenButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.captureScreenButton.Location = new System.Drawing.Point(916, 58);
			this.captureScreenButton.Name = "captureScreenButton";
			this.captureScreenButton.Size = new System.Drawing.Size(75, 40);
			this.captureScreenButton.TabIndex = 8;
			this.captureScreenButton.Text = "Capture Screen";
			this.captureScreenButton.UseVisualStyleBackColor = true;
			this.captureScreenButton.Click += new System.EventHandler(this.captureScreenBtn_Click);
			// 
			// hDivZoomInButton
			// 
			this.hDivZoomInButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.hDivZoomInButton.Location = new System.Drawing.Point(12, 438);
			this.hDivZoomInButton.Name = "hDivZoomInButton";
			this.hDivZoomInButton.Size = new System.Drawing.Size(23, 23);
			this.hDivZoomInButton.TabIndex = 9;
			this.hDivZoomInButton.Text = "+";
			this.hDivZoomInButton.UseVisualStyleBackColor = true;
			this.hDivZoomInButton.Click += new System.EventHandler(this.hDivZoomInButton_Click);
			// 
			// hDivZoomOutButton
			// 
			this.hDivZoomOutButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.hDivZoomOutButton.Location = new System.Drawing.Point(41, 438);
			this.hDivZoomOutButton.Name = "hDivZoomOutButton";
			this.hDivZoomOutButton.Size = new System.Drawing.Size(23, 23);
			this.hDivZoomOutButton.TabIndex = 10;
			this.hDivZoomOutButton.Text = "-";
			this.hDivZoomOutButton.UseVisualStyleBackColor = true;
			this.hDivZoomOutButton.Click += new System.EventHandler(this.hDivZoomOutButton_Click);
			// 
			// vDivZoomOutButton
			// 
			this.vDivZoomOutButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.vDivZoomOutButton.Location = new System.Drawing.Point(1049, 133);
			this.vDivZoomOutButton.Name = "vDivZoomOutButton";
			this.vDivZoomOutButton.Size = new System.Drawing.Size(23, 23);
			this.vDivZoomOutButton.TabIndex = 12;
			this.vDivZoomOutButton.Text = "-";
			this.vDivZoomOutButton.UseVisualStyleBackColor = true;
			this.vDivZoomOutButton.Click += new System.EventHandler(this.vDivZoomOutButton_Click);
			// 
			// vDivZoomInButton
			// 
			this.vDivZoomInButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.vDivZoomInButton.Location = new System.Drawing.Point(1049, 104);
			this.vDivZoomInButton.Name = "vDivZoomInButton";
			this.vDivZoomInButton.Size = new System.Drawing.Size(23, 23);
			this.vDivZoomInButton.TabIndex = 11;
			this.vDivZoomInButton.Text = "+";
			this.vDivZoomInButton.UseVisualStyleBackColor = true;
			this.vDivZoomInButton.Click += new System.EventHandler(this.vDivZoomInButton_Click);
			// 
			// copyScreenButton
			// 
			this.copyScreenButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.copyScreenButton.Enabled = false;
			this.copyScreenButton.Location = new System.Drawing.Point(997, 58);
			this.copyScreenButton.Name = "copyScreenButton";
			this.copyScreenButton.Size = new System.Drawing.Size(75, 40);
			this.copyScreenButton.TabIndex = 14;
			this.copyScreenButton.Text = "Copy Screen";
			this.copyScreenButton.UseVisualStyleBackColor = true;
			this.copyScreenButton.Click += new System.EventHandler(this.copyScreenButton_Click);
			// 
			// copyBufferButton
			// 
			this.copyBufferButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.copyBufferButton.Enabled = false;
			this.copyBufferButton.Location = new System.Drawing.Point(997, 12);
			this.copyBufferButton.Name = "copyBufferButton";
			this.copyBufferButton.Size = new System.Drawing.Size(75, 40);
			this.copyBufferButton.TabIndex = 13;
			this.copyBufferButton.Text = "Copy Buffer";
			this.copyBufferButton.UseVisualStyleBackColor = true;
			this.copyBufferButton.Click += new System.EventHandler(this.copyBufferButton_Click);
			// 
			// Form1
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(1084, 473);
			this.Controls.Add(this.copyScreenButton);
			this.Controls.Add(this.copyBufferButton);
			this.Controls.Add(this.vDivZoomOutButton);
			this.Controls.Add(this.vDivZoomInButton);
			this.Controls.Add(this.hDivZoomOutButton);
			this.Controls.Add(this.hDivZoomInButton);
			this.Controls.Add(this.captureScreenButton);
			this.Controls.Add(this.captureBufferButton);
			this.Name = "Form1";
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
	}
}

