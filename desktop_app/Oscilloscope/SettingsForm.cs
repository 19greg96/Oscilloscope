using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace Oscilloscope {
	public partial class SettingsForm : Form {
		public string SelectedCOMPort { get => ((SerialPortComboboxItem)comPortComboBox.SelectedItem).Name; }

		public class SerialPortComboboxItem {
			public string Name { get; }
			public string Description { get; }
			public string DisplayName { get => Name + " - " + Description; }

			public SerialPortComboboxItem(string name, string description) {
				Name = name;
				Description = description;
			}

			public override string ToString() {
				return Name + " - " + Description;
			}
		}

		public SettingsForm() {
			InitializeComponent();

			comPortComboBox.ValueMember = "Name";
			comPortComboBox.DisplayMember = "DisplayName";
		}

		internal void populateFields() { // TODO: this is only called on form open, if device is added when settings is open, it doesn't update the list
			comPortComboBox.Items.Clear();
			int selectedIndex = -1;
			for (int i = 0; i < Program.getPortList().Count; i ++) {
				SerialPortEnumerator.PortInfo pi = Program.getPortList()[i];
				comPortComboBox.Items.Add(new SerialPortComboboxItem(pi.Name, pi.Description));
				if (pi.Name.Equals(Program.selectedSerialPort)) {
					selectedIndex = i;
				}
			}
			comPortComboBox.SelectedIndex = selectedIndex;
		}

		private void cancelButton_Click(object sender, EventArgs e) {
			this.DialogResult = DialogResult.Cancel;
		}

		private void applyButton_Click(object sender, EventArgs e) {
			this.DialogResult = DialogResult.Yes;
		}
	}
}
