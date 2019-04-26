import com.itextpdf.text.DocumentException;
import com.itextpdf.text.pdf.PdfArray;
import com.itextpdf.text.pdf.PdfDictionary;
import com.itextpdf.text.pdf.PdfName;
import com.itextpdf.text.pdf.PdfNumber;
import com.itextpdf.text.pdf.PdfReader;
import com.itextpdf.text.pdf.PdfStamper;
 
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Locale;
 
public class SplitPage {
 
    public static void main(String[] args) throws Exception {
        if ((args.length!=3)||((!args[2].equals("0"))&&(!args[2].equals("1")))) {
           System.out.println("usage: SplitPage in out half");
           return;
        }
           
        PdfReader reader = new PdfReader(args[0]);
        PdfStamper stamper = new PdfStamper(reader, new FileOutputStream(args[1]));
        int half=(args[2].equals("0")?0:1);

        int n = reader.getNumberOfPages();
        PdfDictionary page;
        PdfArray media;
        for (int p = 1; p <= n; p++) {
            page = reader.getPageN(p);

            media = page.getAsArray(PdfName.MEDIABOX);
            float llx = media.getAsNumber(0).floatValue();
            float lly = media.getAsNumber(1).floatValue();
            float urx = media.getAsNumber(2).floatValue();
            float ury = media.getAsNumber(3).floatValue();
            float w = (urx-llx);
            if (half==0)
               media.set(2,new PdfNumber(llx+(w/2))); else
               media.set(0,new PdfNumber(llx+(w/2)));

/*
            media = page.getAsArray(PdfName.CROPBOX);
            if (media == null) {
                media = page.getAsArray(PdfName.MEDIABOX);
            }
            float llx = media.getAsNumber(0).floatValue();
            float lly = media.getAsNumber(1).floatValue();
            float w = media.getAsNumber(2).floatValue() - media.getAsNumber(0).floatValue();
            float h = media.getAsNumber(3).floatValue() - media.getAsNumber(1).floatValue();

            w=w/2;
            if (half>0) llx=llx+w;

            String command = String.format(Locale.ROOT,
                    "\nq %.2f %.2f %.2f %.2f re W n\nq\n",
                    llx, lly, w, h);

            stamper.getUnderContent(p).setLiteral(command);
            stamper.getOverContent(p).setLiteral("\nQ\nQ\n");
*/
        }
        stamper.close();
        reader.close();
    }
}
