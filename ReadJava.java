import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.*;
import java.nio.ByteOrder;
 
public class ReadJava {

    public static int[] convert(byte buf[]) {
       int intArr[] = new int[buf.length / 4];
       int offset = 0;
       for(int i = 0; i < intArr.length; i++) {
           intArr[i] = (buf[3 + offset] & 0xFF) | 
                       ((buf[2 + offset] & 0xFF) << 8) |
                       ((buf[1 + offset] & 0xFF) << 16) | 
                       ((buf[0 + offset] & 0xFF) << 24);  
           offset += 4;
        }
       return intArr;
    }

    public static int[] convertlittle(byte buf[]) {
       int intArr[] = new int[buf.length / 4];
       int offset = 0;
       for(int i = 0; i < intArr.length; i++) {
           intArr[i] = (buf[3 + offset] & 0xFF) << 24 | 
                       ((buf[2 + offset] & 0xFF) <<16) |
                       ((buf[1 + offset] & 0xFF) << 8) | 
                       ((buf[0 + offset] & 0xFF) << 0);  
           offset += 4;
        }
       return intArr;
    }
 
    public static void main(String[] args) throws Exception {
        RandomAccessFile raf = 
               new RandomAccessFile("test.bin", "r");
	raf.seek(0);
	byte[] bytes = new byte[40];
	raf.read(bytes);
	raf.close();
        int [] intArr = convert(bytes);
        for(int i=0; i<intArr.length; ++i )
          System.out.println(intArr[i]);
    }
}