import tkinter as tk

import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
from matplotlib.figure import Figure

#Const Font variables 
LARGE_FONT = ("Verdana", 12)

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
def Send_Stim_Command(nm_amp, diag_amp, freq):
    print("Stim Cmd Sent...")
    print("NM AMP: ", nm_amp)
    print("DIAG AMP: ", diag_amp)
    print("FREQ: ", freq)

def Send_Stop_Stim_Command():
    print("Stop Stim Cmd Sent...")

def Send_Get_Pos_Data_Command():
    print("Gathered the data...")

class StartPage(tk.Frame):

    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)
        label = tk.Label(self, text = "ERIGO Stimulation Control", font = LARGE_FONT)
        label.grid(row=0,column=0, sticky = 'N', columnspan = 2) #Stick forces text to be "North". columnspan causes the text to bridge 
                                                                 #the columns used for the label/entry pairs

        #Create Label/Entry Objects
        NM_AMP_Entry_Label = tk.Label(self, text = "NM AMP Selection:")
        NM_AMP_Entry = tk.Entry(self)
        Diag_AMP_Entry_Label = tk.Label(self, text = "Diag AMP Selection:")
        Diag_AMP_Entry = tk.Entry(self)
        Freq_Entry_Label = tk.Label(self, text = "Freq Selection:")
        Freq_Entry = tk.Entry(self)

        #Place them in a grid pattern
        NM_AMP_Entry_Label.grid(row=1, column = 0)
        NM_AMP_Entry.grid(row=1, column=1)
        Diag_AMP_Entry_Label.grid(row=2, column = 0)
        Diag_AMP_Entry.grid(row=2, column=1)
        Freq_Entry_Label.grid(row=3, column = 0)
        Freq_Entry.grid(row=3, column=1)

        #Insert default values
        NM_AMP_Entry.insert(0, '0')
        Diag_AMP_Entry.insert(0, '0')
        Freq_Entry.insert(0, '0')

        Stim_Button = tk.Button(self, text = "Start Stim", command = lambda: Send_Stim_Command(NM_AMP_Entry.get(), Diag_AMP_Entry.get(), Freq_Entry.get()))
        Stim_Button.grid(row=4, column=0)

        Stop_Stim_Button = tk.Button(self, text = "Stop Stim", command = lambda: Send_Stop_Stim_Command())
        Stop_Stim_Button.grid(row = 4, column = 1)

        Record_Data_Button = tk.Button(self, text = "Get Position Data", command = lambda: Send_Get_Pos_Data_Command())
        Record_Data_Button.grid(row= 4, column=3)

        Fig = Figure(figsize=(4,4), dpi=100)
        a = Fig.add_subplot(111)
        a.plot([1,2,3,4,5,6,7,9],[1,2,3,4,6,3,4,9])

        canvas = FigureCanvasTkAgg(Fig, self)
        canvas.show()
        canvas.get_tk_widget().grid(row=0, column=3)



        


def main():
   
   App = ErigoGUI()
   App.mainloop()

if __name__ == "__main__":

    main()