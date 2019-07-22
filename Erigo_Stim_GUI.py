import tkinter as tk
from tkinter import ttk


import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

import numpy as np
import sys

from STM32_Serial import *

#Const Font variables 
LARGE_FONT = ("Verdana", 12)

#Plot Constants
PLOT_LWR_LIMIT_X =   0
PLOT_UPR_LIMIT_X = 5000
PLOT_LWR_LIMIT_Y =   0
PLOT_UPR_LIMIT_Y = 4096

class ErigoGUI(tk.Tk):

    def __init__(self,*args, **kwargs):
        
        tk.Tk.__init__(self, *args, **kwargs)
        
        #Acts as a top level frame to place other frames/pages on
        container = tk.Frame(self)
        container.pack(side="top", fill="both", expand = True)

        container.grid_rowconfigure(0, weight=1)
        container.grid_columnconfigure(0, weight=1)

        #Tuple of frames that act as "pages"
        self.frames = {}

        #Istantiates the starting page
        frame = StartPage(container, self)

        self.frames[StartPage] = frame

        frame.grid(row=0,column=0, sticky="nsew")

        #Brings starting page to top of view
        self.show_frame(StartPage)
        
        print(get_com_ports())
        
        #Connect to first available com port
        com_ports = get_com_ports()
        
        if(com_ports):
            for port in com_ports:
                setupSTM32Serial(port)
                break
        else:
            print("COM ports not available...")


    def show_frame(self, controller):
        frame = self.frames[controller]
        frame.tkraise()


#Place holders. This will be defined in STM32_Serial

class Label_Entry_Frame(tk.Frame):
    """Creates a tk Frame that creates label entry pairs.
       The labels(str) are used to access the the actual label and entry as the key to a python dict.
       (Although if the value entered into the entry is all that is required, there is a get_entry method for that.)
       If the defualt_entries value is going to be used, all entries must be supplied a default value."""
    def __init__(self, parent, label_list, default_entries = []):
        tk.Frame.__init__(self, parent)
        
        self.label_entry_pairs = {}
        self.MAX_ROWS = 6

        for i, label_str in enumerate(label_list):
            self.label_entry_pairs[label_str] = [tk.Label(self, text=label_str), tk.Entry(self, )]
            
            #allows label entries to wrap around
            label_row = i%self.MAX_ROWS  
            label_col = 2*(i//self.MAX_ROWS)

            #place the label on the left of the entry
            self.label_entry_pairs[label_str][0].grid(row = label_row, column = label_col)
            self.label_entry_pairs[label_str][1].grid(row = label_row, column = label_col+1)
            
            #Set default entries
            if(default_entries):
                self.label_entry_pairs[label_str][1].insert(0, default_entries[i])
    
    def get_entry(self, key_str):
        return self.label_entry_pairs[key_str][1].get()



class StartPage(tk.Frame):

    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)
        self.title = tk.Label(self, text = "ERIGO Stimulation Control", font = LARGE_FONT)

        self.COM_sel_box = ttk.Combobox(self, postcommand = self.update_COM_sel_box)
        self.COM_Connect_button = tk.Button(self, text = "Connect", command = lambda: self.connect_to_COM())

        self.text_box = tk.Text(self)
        self.text_box.config(borderwidth=2)
        self.Write_to_TextBox("Erigo Stimulation GUI...")
        self.text_box.config(borderwidth = 2, relief="solid")
        self.text_box.config(height=8) #In lines

        self.ADC_Stats_Max = tk.StringVar()
        self.ADC_Stats_Max.set("Max ADC Pos: --")
        self.ADC_Stats_Min = tk.StringVar()
        self.ADC_Stats_Min.set("Min ADC Pos: --")
        self.ADC_Stats_Rec = tk.StringVar()
        self.ADC_Stats_Rec.set("Rec ThresholdZeroCrossing --")

        #Create Label/Entry Frame
        self.stim_control_labels = ["StimMode", "TestAmp_mA", "TestPreTime_ms", "TestPostTime_ms", "TestTotalPulses", "TestInterCycles", 
                                    "TestCyclePhase", "CondAmp_mA", "CondFreq_Hz", "CondNumOfPulses", "ThresholdZeroCrossing",
                                    ]
        self.label_entry = Label_Entry_Frame(self, self.stim_control_labels, ["0"]*len(self.stim_control_labels))

        self.Stim_Button = tk.Button(self, text = "Start Stim", command = lambda: self.Send_Stim_Command(self.label_entry.get_entry("CondAmp_mA"), 
                                                                                                    self.label_entry.get_entry("TestAmp_mA"), 
                                                                                                    self.label_entry.get_entry("CondFreq_Hz"),
                                                                                                    self.label_entry.get_entry("ThresholdZeroCrossing")))
        self.Stop_Stim_Button = tk.Button(self, text = "Stop Stim", command = lambda: self.Send_Stop_Stim_Command())
        self.Record_Data_Button = tk.Button(self, text = "Get Position Data", command = lambda: self.Update_ADC_Pos())

        self.Max_ADC_Label = tk.Label(self, textvariable = self.ADC_Stats_Max)
        self.Rec_ADC_Label = tk.Label(self, textvariable = self.ADC_Stats_Rec)
        self.Min_ADC_Label = tk.Label(self, textvariable = self.ADC_Stats_Min)

        #Creates a matplotlib Figure and a subplot on it
        self.Fig = Figure(figsize=(4,4), dpi=100)
        self.angle_plot = self.Fig.add_subplot(111)
        self.angle_plot.plot()
        self.angle_plot.set_xlim([PLOT_LWR_LIMIT_X, PLOT_UPR_LIMIT_X])
        self.angle_plot.set_ylim([PLOT_LWR_LIMIT_Y, PLOT_UPR_LIMIT_Y])

        #Canvas is a drawing area for tkinter. We put the matplotlib figure onto the canvas
        self.canvas = FigureCanvasTkAgg(self.Fig, self)
        self.canvas.draw()

        #Put all grid/location setting calls in one spot since so it is easier to see all locations at once. 
        self.title.grid(row=0,column=0, sticky = 'N', columnspan = 2) #Stick forces text to be "North". columnspan causes the text to bridge 
                                                                 #the columns used for the label/entry pairs

        self.Place_Widgets()

    def Place_Widgets(self):
        #Put all grid/location setting calls in one spot since so it is easier to see all locations at once. 
        self.title.grid(row=0,column=0, sticky = 'N', columnspan = 2) #Stick forces text to be "North". columnspan causes the text to bridge 
                                                                 #the columns used for the label/entry pairs
        self.text_box.grid(row=0, column = 0) #The text box can fit in the same cell as the title (I think), b/c
                                              #the cell is big enough to hold both of them without overlap

        self.label_entry.grid(row=1, column = 0)

        self.COM_sel_box.grid(row=2, column = 1)
        self.COM_Connect_button.grid(row = 1, column = 1)
        
        self.Stim_Button.grid(row=6, column=0)
        self.Stop_Stim_Button.grid(row = 6, column = 1)
        self.Record_Data_Button.grid(row= 6, column=4)

        self.Max_ADC_Label.grid(row = 4, column=3)
        self.Rec_ADC_Label.grid(row = 4, column=4)
        self.Min_ADC_Label.grid(row = 4, column=5)

        self.canvas.get_tk_widget().grid(row=0, column=3, columnspan = 3)

    def Write_to_TextBox(self, text):

        self.text_box.config(state=tk.NORMAL)
        self.text_box.insert("end", text + "\n")
        self.text_box.config(state=tk.DISABLED)

        self.text_box.yview(tk.END)
        

    def Update_ADC_Pos(self):

        #data = self.Send_Get_Pos_Data_Command()
        self.Send_Get_Pos_Data_Command()

        #this is added so the data can be gathered x amount of ms later (giving time for the uC to gather it)
        #then TKinter can call the function to gather the data.
        #This helps with not blocking the GUI mainloop
        self.after(1000, self.Get_Pos_Data)
        

    def Send_Stim_Command(self, nm_amp, diag_amp, freq, adc_threshold):
        msg_params = CmdMsgParams(int(diag_amp), int(nm_amp), int(freq))
        
        send_Stim_CMD(msg_params)
         
        self.Write_to_TextBox("Stim Cmd Sent...")
        self.Write_to_TextBox("NM AMP: " + str(nm_amp))
        self.Write_to_TextBox("DIAG AMP: " + str(diag_amp))
        self.Write_to_TextBox("FREQ: " + str(freq))
        self.Write_to_TextBox("ThresholdZeroCrossing " + str(adc_threshold))

    def Send_Stop_Stim_Command(self):
        msg_params = CmdMsgParams()
        send_Stop_Proc_CMD(msg_params)
        self.Write_to_TextBox("Stop Stim Cmd Sent...")

    def Send_Get_Pos_Data_Command(self):
        self.Write_to_TextBox("Sending Cmd...")
        adc_msg_params = CmdMsgParams()
        send_CMD_with_Retries(DATA_LOG_MSG_ID, adc_msg_params)

    def Get_Pos_Data(self):

        #Collect data after ACK from uC
        raw_bin_data, all_data_gathered = get_Data()
    
        #----Since all the data is gathered at this point, there is no need to take speed into consideration for the Serial Port.        
        data_ADC1, data_ADC2 = process_raw_serial_data(raw_bin_data, BYTES_PER_DATA_ITEM)

        # #data collected from uC
        # theta = np.arange(0.0, 1000.0, 1.0)
        # y = 750.0*np.sin((0.1/np.pi)*theta)+3000.0 + np.random.normal(0.0, 25.0, theta.size)
        # data = np.vstack((theta, y))
        self.Write_to_TextBox("Gathered the data...")

        #if data is valid...
        x_data = list(range(len(data_ADC1)))
        
        self.angle_plot.clear()

        #Scatter Plot for Raw Data
        self.angle_plot.plot(x_data, data_ADC1, color="blue")
        self.angle_plot.plot(x_data, data_ADC2, color="green")
        self.angle_plot.set_xlim([PLOT_LWR_LIMIT_X, PLOT_UPR_LIMIT_X])
        self.angle_plot.set_ylim([PLOT_LWR_LIMIT_Y, PLOT_UPR_LIMIT_Y])
        #self.angle_plot.set_xlim([0.0, 1000.0])
        #self.angle_plot.set_ylim([min(data_ADC1), max(data_ADC1)])

        #Processing on Data (if any more data processing were to be done, I would consider making it a seperate module)
        max_90_percentile = np.percentile(data_ADC2, 90.0)
        min_10_percentile = np.percentile(data_ADC2, 10.0)
        mid_50_percentile = np.percentile(data_ADC2, 50.0)
        AVG = (max_90_percentile+min_10_percentile)/2

        self.ADC_Stats_Max.set("Max ADC Pos: " + str(int(max_90_percentile)))
        self.ADC_Stats_Min.set("Min ADC Pos: " + str(int(min_10_percentile)))
        self.ADC_Stats_Rec.set("Rec ThresholdZeroCrossing " + str(int(AVG)))

        self.angle_plot.hlines(max_90_percentile, PLOT_LWR_LIMIT_X, PLOT_UPR_LIMIT_X)
        self.angle_plot.hlines(min_10_percentile, PLOT_LWR_LIMIT_X, PLOT_UPR_LIMIT_X)
        self.angle_plot.hlines(AVG, PLOT_LWR_LIMIT_X, PLOT_UPR_LIMIT_X, colors = "red")

        self.canvas.draw()


    def update_COM_sel_box(self):
        list_values = get_com_ports()
        self.COM_sel_box['values'] = list_values

    def connect_to_COM(self):
        com_port = self.COM_sel_box.get()
        if(com_port):
            setupSTM32Serial(com_port)
        else:
            print("COM port not available...")




        


def main():
   
   App = ErigoGUI()
   App.mainloop()

if __name__ == "__main__":

    main()