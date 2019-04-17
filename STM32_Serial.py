import serial
import time


from min import MINTransportSerial

START_BYTE = b'\x02' #ASCII Start of Text
DELIMITER = b'\xF1'  #
END_BYTE = b'\x03'   #ASCII End of Text

SEND_ATTEMPS = 5

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

def send_CMD(cmd):
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

    #There should only be one frame, but just in case/wait for ACK returns a lit of frames
    for frame in frames:
        if frame.min_id == cmd:
            print("Success!!!")
            return True
        else:   
            print("Failure")
            return False

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
        COMM_SUCC = send_CMD(CMD)

    print("Comm was a Succ")


    



    