package com.example.testnfc;

import android.nfc.Tag;
import android.nfc.tech.NfcV;

//import android.util.Log;

public class NFCCommand {

    //***********************************************************************/
    //* the function send an Inventory command (0x26 0x01 0x00)
    //* the argument myTag is the intent triggered with the TAG_DISCOVERED
    //***********************************************************************/
    public static byte[] SendInventoryCommand(Tag myTag) {
        byte[] UIDFrame = new byte[]{(byte) 0x26, (byte) 0x01, (byte) 0x00};
        byte[] response = new byte[]{(byte) 0x01};

        int errorOccured = 1;
        while (errorOccured != 0) {
            try {
                NfcV nfcvTag = NfcV.get(myTag);
                nfcvTag.close();
                nfcvTag.connect();
                response = nfcvTag.transceive(UIDFrame);
                nfcvTag.close();
                if (response[0] == (byte) 0x00) {
                    //Used for DEBUG : Log.i("NFCCOmmand", "SENDED Frame : " + Helper.ConvertHexByteToString((byte) 0x26) + " " + Helper.ConvertHexByteToString((byte) 0x01) + " " + Helper.ConvertHexByteToString((byte) 0x00) );
                    errorOccured = 0;
                }
            } catch (Exception e) {
                errorOccured++;
                //Used for DEBUG : Log.i("Polling**ERROR***", "SendInventoryCommand" + Integer.toString(errorOccured));
                if (errorOccured >= 2) {
                    //Used for DEBUG : Log.i("Exception","Inventory Exception " + e.getMessage());
                    return response;
                }
            }
        }
        //Used for DEBUG : Log.i("NFCCOmmand", "Response " + Helper.ConvertHexByteToString((byte)response[0]));
        return response;
    }


    //***********************************************************************/
    //* the function send an ReadSingle command (0x0A 0x20) || (0x02 0x20)
    //* the argument myTag is the intent triggered with the TAG_DISCOVERED
    //* example : StartAddress {0x00, 0x02}  NbOfBlockToRead : {0x04}
    //* the function will return 04 blocks read from address 0002
    //* According to the ISO-15693 maximum block read is 32 for the same sector
    //***********************************************************************/
    public static byte[] SendReadSingleBlockCommand(Tag myTag, byte[] StartAddress) {
        byte[] response = new byte[]{(byte) 0x0A};
        byte[] ReadSingleBlockFrame;

        ReadSingleBlockFrame = new byte[]{(byte) 0x0A, (byte) 0x20, StartAddress[1], StartAddress[0]};

        int errorOccured = 1;
        while (errorOccured != 0) {
            try {
                NfcV nfcvTag = NfcV.get(myTag);
                nfcvTag.close();
                nfcvTag.connect();
                response = nfcvTag.transceive(ReadSingleBlockFrame);
                if (response[0] == (byte) 0x00) {
                    errorOccured = 0;
                    //Used for DEBUG : Log.i("NFCCOmmand", "SENDED Frame : " + Helper.ConvertHexByteArrayToString(ReadSingleBlockFrame));
                }
            } catch (Exception e) {
                errorOccured++;
                //Used for DEBUG : Log.i("NFCCOmmand", "Response Read Single Block" + Helper.ConvertHexByteArrayToString(response));
                if (errorOccured == 2) {
                    //Used for DEBUG : Log.i("Exception","Exception " + e.getMessage());
                    return response;
                }
            }
        }
        //Used for DEBUG : Log.i("NFCCOmmand", "Response Read Sigle Block" + Helper.ConvertHexByteArrayToString(response));
        return response;
    }


    //***********************************************************************/
    //* the function send an ReadSingle Custom command (0x0A 0x20) || (0x02 0x20)
    //* the argument myTag is the intent triggered with the TAG_DISCOVERED
    //* example : StartAddress {0x00, 0x02}  NbOfBlockToRead : {0x04}
    //* the function will return 04 blocks read from address 0002
    //* According to the ISO-15693 maximum block read is 32 for the same sector
    //***********************************************************************/
    public static byte[] SendReadMultipleBlockCommandCustom(Tag myTag, byte[] StartAddress, byte NbOfBlockToRead) {
        long cpt = 0;
        boolean checkCorrectAnswer = true;

        //int NbBytesToRead = (NbOfBlockToRead*4)+1;
        int NbBytesToRead = NbOfBlockToRead * 4;
        byte[] FinalResponse = new byte[NbBytesToRead + 1];

        for (int i = 0; i <= (NbOfBlockToRead * 4) - 4; i = i + 4) {
            byte[] temp = new byte[5];
            int incrementAddressStart0 = (StartAddress[0] + i / 256);                                //Most Important Byte
            int incrementAddressStart1 = (StartAddress[1] + i / 4) - (incrementAddressStart0 * 255);    //Less Important Byte

            temp = null;
            while (temp == null || temp[0] == 1 && cpt <= 2) {
                temp = SendReadSingleBlockCommand(myTag, new byte[]{(byte) incrementAddressStart0, (byte) incrementAddressStart1});
                cpt++;
            }
            cpt = 0;

            //Check  if Read problem
            if (temp[0] != 0x00)
                checkCorrectAnswer = false;

            if (i == 0) {
                for (int j = 0; j <= 4; j++) {
                    if (temp[0] == 0x00)
                        FinalResponse[j] = temp[j];
                    else
                        FinalResponse[j] = (byte) 0xFF;
                }
            } else {
                for (int j = 1; j <= 4; j++) {
                    if (temp[0] == 0x00)
                        FinalResponse[i + j] = temp[j];
                    else
                        FinalResponse[j] = (byte) 0xFF;
                }
            }
        }

        if (checkCorrectAnswer == false)
            FinalResponse[0] = (byte) 0xAE;

        return FinalResponse;
    }


    //***********************************************************************/
    //* the function send an WriteSingle command (0x0A 0x21) || (0x02 0x21)
    //* the argument myTag is the intent triggered with the TAG_DISCOVERED
    //* example : StartAddress {0x00, 0x02}  DataToWrite : {0x04 0x14 0xFF 0xB2}
    //* the function will write {0x04 0x14 0xFF 0xB2} at the address 0002
    //***********************************************************************/
    public static byte[] SendWriteSingleBlockCommand (Tag myTag, byte[] StartAddress, byte[] DataToWrite)
    {
        byte[] response = new byte[] {(byte) 0xFF};
        byte[] WriteSingleBlockFrame;

        WriteSingleBlockFrame = new byte[]{(byte) 0x0A, (byte) 0x21, StartAddress[1], StartAddress[0],
                                            DataToWrite[0], DataToWrite[1], DataToWrite[2], DataToWrite[3]};

        int errorOccured = 1;
        while(errorOccured != 0)
        {
            try
            {
                NfcV nfcvTag = NfcV.get(myTag);
                nfcvTag.close();
                nfcvTag.connect();
                response = nfcvTag.transceive(WriteSingleBlockFrame);
                if(response[0] == (byte) 0x00)
                {
                    errorOccured = 0;
                    //Used for DEBUG : Log.i("*******", "**SUCCESS** Write Data " + DataToWrite[0] +" "+ DataToWrite[1] +" "+ DataToWrite[2] +" "+ DataToWrite[3] + " at address " +  (byte)StartAddress[0] +" "+ (byte)StartAddress[1]);
                }
            }
            catch(Exception e)
            {
                errorOccured++;
                //Used for DEBUG : Log.i("NFCCOmmand", "Write Single block command  " + errorOccured);
                if(errorOccured == 2)
                {
                    //Used for DEBUG : Log.i("Exception","Exception " + e.getMessage());
                    //Used for DEBUG : Log.i("WRITE", "**ERROR WRITE SINGLE** at address " +  Helper.ConvertHexByteArrayToString(StartAddress));
                    return response;
                }
            }
        }
        return response;
    }
}