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
    
    for i in range(SEND_ATTEMPS):
        min_handler.send_frame(1, (3).to_bytes(1, byteorder="big") + (3).to_bytes(1, byteorder="big") + (3).to_bytes(1, byteorder="big") + (3).to_bytes(1, byteorder="big"))
        time.sleep(1)
        
    