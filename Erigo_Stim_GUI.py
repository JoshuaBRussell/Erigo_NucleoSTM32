import tkinter as tk

import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
from matplotlib.figure import Figure

import numpy as np

#Const Font variables 
LARGE_FONT = ("Verdana", 12)

#Plot Constants
PLOT_LWR_LIMIT_X =   0
PLOT_UPR_LIMIT_X = 1000
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

    def show_frame(self, controller):
        frame = self.frames[controller]
        frame.tkraise()


#Place holders. This will be defined in STM32_Serial
def Send_Stim_Command(nm_amp, diag_amp, freq, adc_threshold):
    print("Stim Cmd Sent...")
    print("NM AMP: ", nm_amp)
    print("DIAG AMP: ", diag_amp)
    print("FREQ: ", freq)
    print("ADC THRESHOLD: ", adc_threshold)

def Send_Stop_Stim_Command():
    print("Stop Stim Cmd Sent...")

def Send_Get_Pos_Data_Command():
    print("Sending Cmd...")

    #data collected from uC
    theta = np.arange(0.0, 1000.0, 1.0)
    y = 750.0*np.sin((0.1/np.pi)*theta)+3000.0 + np.random.normal(0.0, 25.0, theta.size)
    data = np.vstack((theta, y))
    print("Gathered the data...")
    print("Data shape: ", data.shape)
    return data

class Label_Entry_Frame(tk.Frame):
    """Creates a tk Frame that creates label entry pairs.
       The labels(str) are used to access the the actual label and entry as the key to a python dict.
       (Although if the value entered into the entry is all that is required, there is a get_entry method for that.)
       If the defualt_entries value is going to be used, all entries must be supplied a default value."""
    def __init__(self, parent, label_list, default_entries = []):
        tk.Frame.__init__(self, parent)
        
        self.label_entry_pairs = {}

        for i, label_str in enumerate(label_list):
            self.label_entry_pairs[label_str] = [tk.Label(self, text=label_str), tk.Entry(self, )]

            #place the label on the left of the entry
            self.label_entry_pairs[label_str][0].grid(row = i, column = 0)
            self.label_entry_pairs[label_str][1].grid(row = i, column = 1)
            
            #Set default entries
            if(default_entries):
                self.label_entry_pairs[label_str][1].insert(0, default_entries[i])
    
    def get_entry(self, key_str):
        return self.label_entry_pairs[key_str][1].get()



class StartPage(tk.Frame):

    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)
        self.title = tk.Label(self, text = "ERIGO Stimulation Control", font = LARGE_FONT)

        self.ADC_Stats_Max = tk.StringVar()
        self.ADC_Stats_Max.set("Max ADC Pos: --")
        self.ADC_Stats_Min = tk.StringVar()
        self.ADC_Stats_Min.set("Min ADC Pos: --")
        self.ADC_Stats_Rec = tk.StringVar()
        self.ADC_Stats_Rec.set("Rec ADC Threshold: --")

        #Create Label/Entry Frame
        self.stim_control_labels = ["NM AMP Selection:", "Diag AMP Selection:", "Freq Selection:", "ADC Threshold:"]
        self.label_entry = Label_Entry_Frame(self, self.stim_control_labels, ["0"]*len(self.stim_control_labels))

        self.Stim_Button = tk.Button(self, text = "Start Stim", command = lambda: Send_Stim_Command(self.label_entry.get_entry("NM AMP Selection:"), 
                                                                                                    self.label_entry.get_entry("Diag AMP Selection:"), 
                                                                                                    self.label_entry.get_entry("Freq Selection:"),
                                                                                                    self.label_entry.get_entry("ADC Threshold:")))
        self.Stop_Stim_Button = tk.Button(self, text = "Stop Stim", command = lambda: Send_Stop_Stim_Command())
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
        self.canvas.show()

        #Put all grid/location setting calls in one spot since so it is easier to see all locations at once. 
        self.title.grid(row=0,column=0, sticky = 'N', columnspan = 2) #Stick forces text to be "North". columnspan causes the text to bridge 
                                                                 #the columns used for the label/entry pairs

        self.Place_Widgets()

    def Place_Widgets(self):
        self.label_entry.grid(row=1, column = 0)
        
        self.Stim_Button.grid(row=5, column=0)
        self.Stop_Stim_Button.grid(row = 5, column = 1)
        self.Record_Data_Button.grid(row= 5, column=4)

        self.Max_ADC_Label.grid(row = 3, column=3)
        self.Rec_ADC_Label.grid(row = 3, column=4)
        self.Min_ADC_Label.grid(row = 3, column=5)

        self.canvas.get_tk_widget().grid(row=0, column=3, columnspan = 3)

    def Update_ADC_Pos(self):

        data = Send_Get_Pos_Data_Command()
        
        #if data is valid...
        x_data = data[0, :]
        y_data = data[1, :]
        
        self.angle_plot.clear()

        #Scatter Plot for Raw Data
        self.angle_plot.scatter(x_data, y_data, color="blue")
        self.angle_plot.set_xlim([PLOT_LWR_LIMIT_X, PLOT_UPR_LIMIT_X])
        self.angle_plot.set_ylim([PLOT_LWR_LIMIT_Y, PLOT_UPR_LIMIT_Y])
        #self.angle_plot.set_xlim([0.0, 1000.0])
        #self.angle_plot.set_ylim([min(y_data), max(y_data)])

        #Processing on Data (if any more data processing were to be done, I would consider making it a seperate module)
        max_90_percentile = np.percentile(y_data, 90.0)
        min_10_percentile = np.percentile(y_data, 10.0)
        mid_50_percentile = np.percentile(y_data, 50.0)
        AVG = (max_90_percentile+min_10_percentile)/2

        print("MAX: ", max_90_percentile)
        print("AVG: ", AVG)
        print("MIN: ", min_10_percentile)
        print("MID: ", mid_50_percentile)

        self.ADC_Stats_Max.set("Max ADC Pos: " + str(int(max_90_percentile)))
        self.ADC_Stats_Min.set("Min ADC Pos: " + str(int(min_10_percentile)))
        self.ADC_Stats_Rec.set("Rec ADC Threshold: " + str(int(AVG)))

        self.angle_plot.hlines(max_90_percentile, PLOT_LWR_LIMIT_X, PLOT_UPR_LIMIT_X)
        self.angle_plot.hlines(min_10_percentile, PLOT_LWR_LIMIT_X, PLOT_UPR_LIMIT_X)
        self.angle_plot.hlines(AVG, PLOT_LWR_LIMIT_X, PLOT_UPR_LIMIT_X, colors = "red")

        self.canvas.draw()




        


def main():
   
   App = ErigoGUI()
   App.mainloop()

if __name__ == "__main__":

    main()