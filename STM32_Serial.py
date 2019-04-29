import serial
import time

import matplotlib.pyplot as plt
from min import MINTransportSerial

START_BYTE = b'\x02' #ASCII Start of Text
DELIMITER = b'\xF1'  #
END_BYTE = b'\x03'   #ASCII End of Text

SEND_ATTEMPS = 5

#Msg IDs
END_OF_DATA_ID = 3

BYTES_PER_DATA_ITEM = 4

TEST_AMPLITUDE_MA  = 300
NM_AMPITUDE_MA = 80
FREQ_SEL = 3 #0/1/2/3 <->12.5/25/50/100Hz  

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

def send_CMD(min_handler: MINTransportSerial, cmd):
    """Sends a MIN frame with the CMD as the MIN identifier byte. Then waits for an ACK from the uC with the same CMD."""
    #junk data at the moment
    data = b''
    data_array = [100, 50, 3]
    for i in range(len(data_array)):
        data+=data_array[i].to_bytes(2, byteorder="big")
    
    #Send CMD
    print("Send CMD...")
    min_handler.send_frame(cmd, data)
    
    #Wait for ACK from uC
    print("Waiting for ACK...")
    frames = wait_for_ACK(min_handler, timeout = 1.0)

    #There should only be one frame, but just in case/"wait for ACK" returns a lit of frames
    for frame in frames:
        if frame.min_id == cmd:
            print("ACK Received...")
            return True
        else:   
            print("ACK Failure...Wrong ACK CMD type returned...")
            return False


def get_Data(min_handler: MINTransportSerial):
    data_xfer_complete = False
    raw_bin_data = b''
    while(not(data_xfer_complete)):
        #Polls to see if MIN frames have been received 
        rx_frames = wait_for_frames(min_handler)

        #Look at the frames to see if an "End of Data" has been sent
        #If not, keep polling. Ignores any payload data after "End of Data" Msg.
        for frame in rx_frames:
            if(frame.min_id == END_OF_DATA_ID):
                data_xfer_complete = True
                break #Ensure that data retrieved of/after the final data cmd message is not considered part of the data

            raw_bin_data += frame.payload

    return raw_bin_data

def process_raw_serial_data(raw_bin_data, num_bytes_per_int):
    """Takes individual bytes and concatenates them into int type based on num_bytes_per_int"""
    data = []
    for i in range(len(raw_bin_data)//(num_bytes_per_int)):
        i = num_bytes_per_int*i
        data.append(int.from_bytes(raw_bin_data[i:i+num_bytes_per_int], byteorder='big'))

    return data

def main(Test_Amplitude_mA, NM_Amplitude_mA, Freq_Sel):
    ser = serial.Serial("COM3", 115200, timeout = 1.0)
    for i in range(5):
        #sdata = START_BYTE + (Test_Amplitude_mA).to_bytes(2, byteorder = 'big') + DELIMITER + (NM_Amplitude_mA).to_bytes(2, byteorder = 'big') + DELIMITER + (Freq_Sel).to_bytes(2, byteorder = 'big') + END_BYTE
        data_list = [170, 170, 170, 1, 4, 3, 1, 1, 3, 28, 123, 113, 228, 85]
        data = b''
        for i in range(len(data_list)):
            data += (data_list[i].to_bytes(1, byteorder='big'))
        print(data)
        print("Sending Data...%d", i)
        print(ser.write(data))
        print("READ: ", ser.read(16))
        time.sleep(1)


    ser.close()


if __name__ == "__main__":
    #main(TEST_AMPLITUDE_MA, NM_AMPITUDE_MA, FREQ_SEL)

    min_handler = MINTransportSerial("COM3", 115200)
    
    CMD = 2
    COMM_SUCC = False
    while(not(COMM_SUCC)):
        print()
        print()
        COMM_SUCC = send_CMD(min_handler, CMD)

    #Collect data after ACK from uC
    raw_bin_data = get_Data(min_handler)
    print(raw_bin_data)
 
    #----Since all the data is gathered at this point, there is no need to take speed into consideration for the Serial Port.        
    data = process_raw_serial_data(raw_bin_data, BYTES_PER_DATA_ITEM)
    print("len:", len(data))

    #Analyze Data
    print("ADC Data Collected.")
    
    #Display Analog Data
    plt.plot(data)
    plt.show()

    print("Comm was a Succ")


    



    