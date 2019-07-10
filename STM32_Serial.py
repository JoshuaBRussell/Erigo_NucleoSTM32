import serial
import time
import serial.tools.list_ports

import matplotlib.pyplot as plt
from min import MINTransportSerial

#Msg IDs
NO_MSG_ID = 0
STIM_CON_MSG_ID = 1
DATA_LOG_MSG_ID = 2
END_OF_DATA_ID = 3
STOP_PROC_ID = 4

BYTES_PER_DATA_ITEM = 2

TEST_AMPLITUDE_MA  = 300
NM_AMPITUDE_MA = 80
FREQ_SEL = 3 #0/1/2/3 <->12.5/25/50/100Hz

_min_handler = None

class CmdMsgParams():

    def __init__(self,diag_amp_ma=0, nm_amp_ma=0, freq_sel=0):
        self.diag_amp_ma = diag_amp_ma
        self.nm_amp_ma = nm_amp_ma
        self.freq_sel = freq_sel
        

def wait_for_ACK(min_handler: MINTransportSerial, timeout = 0.1):
    start = time.time()
    while True:
        frames = min_handler.poll()
        if frames:
            return frames
        
        #Return an empty frames after timeout
        if time.time() - start >= timeout:
            return frames

def wait_for_frames(min_handler: MINTransportSerial):
    while True:
        # The polling will generally block waiting for characters on a timeout
        # How much CPU time this takes depends on the Python serial implementation
        # on the target machine
        frames = min_handler.poll()
        if frames:
            return frames

def send_CMD(cmd_id, cmd_msg_params):
    """Sends a MIN frame with the CMD as the MIN identifier byte. Then waits for an ACK from the uC with the same CMD.
       Returns False if no ACK was received within timeout or if the wrong cmd_id was returned as an ACK"""
    
    data = b''
    data+=cmd_msg_params.diag_amp_ma.to_bytes(2, byteorder="big")
    data+=cmd_msg_params.nm_amp_ma.to_bytes(2, byteorder="big")
    data+=cmd_msg_params.freq_sel.to_bytes(2, byteorder="big")
    
    #Send CMD
    print("Send CMD...")
    _min_handler.send_frame(cmd_id, data)
    
    #Wait for ACK from uC
    print("Waiting for ACK...")
    frames = wait_for_ACK(_min_handler, timeout = 1.0)

    #There should only be one frame, but just in case/"wait for ACK" returns a list of frames
    for frame in frames:
        if frame.min_id == cmd_id:
            print("ACK Received...")
            return True
        else:   
            print("ACK Failure...Wrong ACK CMD type returned...")
            return False
    #Incdicates no ACK ever happened
    return False


def send_CMD_with_Retries(cmd_id, cmd_msg_params, num_of_retry_attempts=5):
    """Keeps sending CMD messages until an ACK is received or until num_of_retry_attempts messages have been
       sent with no repsonse. Returns True if an ACK was received, returns false otherwise."""
    comm_ack = False
    msg_send_attemps = 0
    while(msg_send_attemps<num_of_retry_attempts and not(comm_ack)):
        msg_send_attemps+=1
        comm_ack = send_CMD(cmd_id, cmd_msg_params)
    
    return comm_ack

def get_Data():

    rx_frames = wait_for_ACK(_min_handler, 15.0)#Timeout should be long enough to allow for the data to be collected 
                                               #plus some for it to be sent

    data_xfer_complete = False
    raw_bin_data = b''
    if(rx_frames):
        while(not(data_xfer_complete)):

            #Look at the frames to see if an "End of Data" has been sent
            #If not, keep polling. Ignores any payload data after "End of Data" Msg.
            for frame in rx_frames:
                if(frame.min_id == END_OF_DATA_ID):
                    data_xfer_complete = True
                    break #Ensure that data retrieved of/after the final data cmd message is not considered part of the data

                raw_bin_data += frame.payload

            rx_frames = wait_for_ACK(_min_handler, 0.5) #Gather frames that come in after the the initial poll. The timeout is much 
                                                       #less since it doesn't have to account for the time to actually gather the data
            
            #This is just so if a timeout occurs, the while loop stops.
            if(not(rx_frames)):
                break

    return raw_bin_data, data_xfer_complete 

def process_raw_serial_data(raw_bin_data, num_bytes_per_int):
    """Takes individual bytes and concatenates them into int type based on num_bytes_per_int.
       Since two ADC's are being used, every other data item is for a specific ADC.          """
    data = []
    for i in range(len(raw_bin_data)//(num_bytes_per_int)):
        i = num_bytes_per_int*i

        data.append(int.from_bytes(raw_bin_data[i:i+num_bytes_per_int], byteorder='big'))

    return data[0::2], data[1::2] #Returns even elements for one ADC, and odd for the other

def send_ADC_CMD(adc_msg_params):
    send_CMD_with_Retries(DATA_LOG_MSG_ID, adc_msg_params)

    #Collect data after ACK from uC
    raw_bin_data, all_data_gathered = get_Data()
 
    #----Since all the data is gathered at this point, there is no need to take speed into consideration for the Serial Port.        
    data = process_raw_serial_data(raw_bin_data, BYTES_PER_DATA_ITEM)
    print("len:", len(data))

    #Analyze Data
    print("ADC Data Collected.")
    
    #Display Analog Data
    plt.plot(data)
    plt.show()


def send_Stim_CMD(stim_msg_params):
    send_CMD_with_Retries(STIM_CON_MSG_ID, stim_msg_params)


def send_Stop_Proc_CMD(stop_proc_msg_params):
    send_CMD_with_Retries(STOP_PROC_ID, stop_proc_msg_params)



def setupSTM32Serial(com_port):
    global _min_handler
    try:
        _min_handler = MINTransportSerial(com_port, 115200)
        print("Connected to COM Port:", com_port)
    except:
        print("Either COM doesn't exist or has already been connected to...", com_port)


#Returns a list of available COM ports
def get_com_ports():
    
    com_ports_not_zero = False
    com_ports = serial.tools.list_ports.comports() #Returns a Python Iterable
    com_port_list = []
    
    for port in com_ports:
        com_port_list.append(str(port.device))
    
    return com_port_list


if __name__ == "__main__":

    setupSTM32Serial()
    
    stim_msg_params = CmdMsgParams(100, 50, 3)
    #send_Stim_CMD(stim_msg_params)
    #send_Stop_Proc_CMD(stim_msg_params)
    send_ADC_CMD(stim_msg_params)



    



    