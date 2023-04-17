package com.ankurmaurya;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

public class PEFormatReader {

	public static void main(String[] args) throws IOException {

		File exeFile = new File("D:\\SmartX.exe");
		FileInputStream fis = new FileInputStream(exeFile);
		
		byte[] data = new byte[64];
		fis.read(data);
		
		char[] magicNumber = {(char)data[0], (char)data[1]};
		System.out.println("DOS Header->");
		System.out.println(magicNumber);
		System.out.println("");
		
		
		byte[] newExeAddressBuffer = {data[60],data[61],data[62],data[63]};
		int newExeAddress = fromByteArray(newExeAddressBuffer);
		System.out.println("New EXE Offset Position->");
		System.out.println(newExeAddress);
		System.out.println(String.format("0x%08X", newExeAddress));
		System.out.println("");
		
		int dosStubLength = newExeAddress - 64;
		byte[] dosBuffer = new byte[dosStubLength];
		fis.read(dosBuffer);

		System.out.println("DOS Stub Program->");
		for(int b = 0; b<dosBuffer.length; b++) {
			System.out.print((char)dosBuffer[b]);
		}
		System.out.println("");
		System.out.println("");
		
		byte[] imageNTHeaderBuffer = new byte[24];
		fis.read(imageNTHeaderBuffer);
		
		char[] signature = {(char)imageNTHeaderBuffer[0], (char)imageNTHeaderBuffer[1], (char)imageNTHeaderBuffer[2], (char)imageNTHeaderBuffer[3]};
		System.out.println("Signature->");
		System.out.println(signature);
		System.out.println("");
		
		
		
		
		
		fis.close();

	}
	
	

	public static int fromByteArray(byte[] bytes) {
	     return ((bytes[3] & 0xFF) << 24) | 
	            ((bytes[2] & 0xFF) << 16) | 
	            ((bytes[1] & 0xFF) << 8 ) | 
	            ((bytes[0] & 0xFF) << 0 );
	}

	
}
