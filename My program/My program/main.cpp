//
//  main.cpp
//  My program
//
//  Created by Mark on 2/11/15.
//  Copyright (c) 2015 Mark. All rights reserved.
//

//#include <iostream>

//int main(int argc, const char * argv[]) {
    // insert code here...
  //  std::cout << "Hello, World!\n";
    //return 0;
//}
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
using System.Windows.Forms;

using Emgu.CV;
using Emgu.Util;
using Emgu.CV.Structure;
namespace My_EMGU_Program
{
    public partial class Form1 : Form
    {
        IntPtr pFrame;
        IntPtr pFrImg;
        IntPtr pBkImg;
        //psmallImg用于运动前景的精确度判断
        IntPtr psmallImg;
        
        IntPtr pFrameMat;
        IntPtr pFrMat;
        IntPtr pBkMat;
        
        Image<Bgr, byte> image_frame;
        Image<Gray, byte> image_bk, image_fr;
        
        IntPtr pCapture;
        
        enum Status
        {
            Running,
            Stop,
            Break
        };
        Status status = Status.Running;
        
        int nFrmNum = 0;
        
        double g_precision = 0.001;
        int g_inteval = 10;//为了节省时间，每隔g_inteval帧检测一次变化，这样处理的好处是时间减少，缺点是不精确，尤其是背景变化稍快的场景；
        double fps;
        
        
        
        public Form1()
        {
            InitializeComponent();
            text_inteval.Text = 10+"";
            combo_precision.SelectedIndex = 1;
        }
        
        private void but_reset_Click(object sender, EventArgs e)
        {
            text_inteval.Text = 10 + "";
            combo_precision.SelectedIndex = 1;
            but_start.Enabled = true;
            status = Status.Break;
        }
        
        private void set_precision()
        {
            switch (combo_precision.SelectedIndex)
            {
                case 0: g_precision = 0.0001; break;
                    
                case 2: g_precision = 0.005; break;
                case 3: g_precision = 0.01; break;
                    
                default: g_precision = 0.001; break;
            }
        }
        
        private void but_avi_Click(object sender, EventArgs e)
        {
            OpenFileDialog avi = new OpenFileDialog();
            avi.InitialDirectory = Environment.CurrentDirectory;
            avi.Filter = "*.avi|*.avi|*.rmvb|*.rmvb|*.*|*.*";
            if (avi.ShowDialog() == DialogResult.OK)
            {
                text_filename.Text = avi.FileName;
            }
        }
        
        private void but_txt_Click(object sender, EventArgs e)
        {
            SaveFileDialog record = new SaveFileDialog();
            record.InitialDirectory = Environment.CurrentDirectory;
            record.Filter = "*.txt|*.txt";
            if (record.ShowDialog() == DialogResult.OK)
            {
                text_record.Text = record.FileName;
            }
        }
        
        private int analyse(int height,int width,IntPtr image)
        {
            int row, col;
            MCvScalar s;
            double val;
            int record_point = 0;
            
            const int pix_step = 5;
            
            double all_point = (height * width * 1.0);//像素面积
            double cmp_result = g_precision / (pix_step * 1.0);//检测精度控制
            
            for (row = 0; row < height; row += pix_step)
                for (col = 0; col < width; col += pix_step)
                {
                    s = CvInvoke.cvGet2D(image, row, col);
                    val = s.v0;
                    if (val != .0)
                    {
                        record_point += 1;
                        //return nFrmNum;
                        if (record_point / all_point > cmp_result)
                        {
                            return nFrmNum;
                        }
                        
                    }
                }
            
            return -1;
        }
        
        private IntPtr inteval_capture(IntPtr capture, int inteval)
        {
            int i = 0;
            IntPtr cur_frame;
            for (i = 0; i < inteval - 1; i++)
            {
                cur_frame = CvInvoke.cvQueryFrame(capture);
                if (cur_frame==IntPtr.Zero)
                    return IntPtr.Zero;
            }
            return CvInvoke.cvQueryFrame(capture);
        }
        
        private string calc_time(int cur_frame, int inteval)
        {
            
            double all_time = cur_frame * inteval * 1.0 / fps;
            long int_time = (long)all_time;
            long hour, min;
            hour = int_time / 3600;
            int_time = int_time % 3600;
            min = int_time / 60;
            int_time = int_time % 60;
            return hour + ":" + min + ":" + int_time;
            
        }
        
        private void begin()
        {
            but_avi.Enabled = false;
            but_txt.Enabled = false;
            but_reset.Enabled = false;
            but_start.Enabled = false;
            nFrmNum = 0;
            set_precision();
            listBox1.Items.Clear();
            g_inteval = int.Parse(text_inteval.Text);
            status = Status.Running;
            
        }
        private void over()
        {
            but_avi.Enabled = true;
            but_txt.Enabled = true;
            but_reset.Enabled = true;
            listBox1.Items.Add("视频检测结束");
            
        }
        
        private void but_start_Click(object sender, EventArgs e)
        {
            begin();
            int cur_frame = -1, last_frame = -1;
            float scale = 0.3f;
            
            //获取视频流
            pCapture  = CvInvoke.cvCreateFileCapture(text_filename.Text);
            if (pCapture ==IntPtr.Zero)
            {
                MessageBox.Show("视频文件有误或者格式不支持，请使用avi格式");
                return;
            }
            
            using (FileStream afile = new FileStream(text_record.Text, FileMode.Create))
            {
                using (StreamWriter writer = new StreamWriter(afile))
                {
                    int all_frame = (int)CvInvoke.cvGetCaptureProperty(pCapture,Emgu.CV.CvEnum.CAP_PROP.CV_CAP_PROP_FRAME_COUNT);
                    
                    fps = CvInvoke.cvGetCaptureProperty(pCapture,Emgu.CV.CvEnum.CAP_PROP.CV_CAP_PROP_FPS);
                    
                    
                    int p_width = (int)CvInvoke.cvGetCaptureProperty(pCapture, Emgu.CV.CvEnum.CAP_PROP.CV_CAP_PROP_FRAME_WIDTH);
                    int p_height = (int)CvInvoke.cvGetCaptureProperty(pCapture, Emgu.CV.CvEnum.CAP_PROP.CV_CAP_PROP_FRAME_HEIGHT);
                    Emgu.CV.CvEnum.IPL_DEPTH p_depth = Emgu.CV.CvEnum.IPL_DEPTH.IPL_DEPTH_8U;
                    image_frame = new Image<Bgr, byte>(p_width, p_height);
                    image_bk = new Image<Gray, byte>(p_width, p_height);
                    image_fr = new Image<Gray, byte>(p_width,p_height);
                    string str_cur_frame = string.Empty;
                    string str_last_frame = string.Empty;
                    bool is_write = false;
                    
                    while (status!=Status.Break)
                    {
                        pFrame = inteval_capture(pCapture,g_inteval);
                        if (pFrame == IntPtr.Zero)
                        {
                            over();
                            status = Status.Stop;
                            while (true)
                            {
                                
                                System.Windows.Forms.Application.DoEvents();
                                if (status != Status.Stop)
                                    break;
                            }
                            continue;
                            
                        }
                        nFrmNum++;
                        
                        if (nFrmNum == 1)
                        {
                            
                            psmallImg = CvInvoke.cvCreateImage(new Size((int)(p_width*scale),(int)(p_height*scale)),Emgu.CV.CvEnum.IPL_DEPTH.IPL_DEPTH_8U,1);
                            
                            pBkImg = CvInvoke.cvCreateImage(new Size(p_width, p_height),p_depth,1);
                            pFrImg = CvInvoke.cvCreateImage(new Size(p_width, p_height), p_depth, 1);
                            
                            pBkMat = CvInvoke.cvCreateMat(p_height,p_width, Emgu.CV.CvEnum.MAT_DEPTH.CV_32F);
                            pFrMat = CvInvoke.cvCreateMat( p_height,p_width, Emgu.CV.CvEnum.MAT_DEPTH.CV_32F);
                            pFrameMat = CvInvoke.cvCreateMat( p_height,p_width, Emgu.CV.CvEnum.MAT_DEPTH.CV_32F);
                            
                            //转化成单通道图像再处理
                            CvInvoke.cvCvtColor(pFrame, pBkImg, Emgu.CV.CvEnum.COLOR_CONVERSION.CV_BGR2GRAY);
                            CvInvoke.cvCvtColor(pFrame, pFrImg, Emgu.CV.CvEnum.COLOR_CONVERSION.CV_BGR2GRAY);
                            
                            CvInvoke.cvConvert(pFrImg, pFrameMat);
                            CvInvoke.cvConvert(pFrImg, pFrMat);
                            CvInvoke.cvConvert(pFrImg, pBkMat);
                            
                        }
                        else
                        {
                            
                            CvInvoke.cvCvtColor(pFrame, pFrImg, Emgu.CV.CvEnum.COLOR_CONVERSION.CV_BGR2GRAY);
                            CvInvoke.cvConvert(pFrImg, pFrameMat);
                            
                            //当前帧跟背景图相减
                            CvInvoke.cvAbsDiff(pFrameMat, pBkMat, pFrMat);
                            
                            //更新背景图
                            CvInvoke.cvRunningAvg(pFrameMat, pBkMat, 0.003, IntPtr.Zero);
                            //将背景转化为图像格式，用以显示
                            CvInvoke.cvConvert(pBkMat, pBkImg);
                            
                            //二值化前景图
                            CvInvoke.cvThreshold(pFrMat, pFrImg, 60, 255,Emgu.CV.CvEnum.THRESH.CV_THRESH_BINARY);
                            
                            //进行形态学滤波，去掉噪音
                            //CvInvoke.cvErode(pFrImg, pFrImg, IntPtr.Zero, 1);
                            //CvInvoke.cvDilate(pFrImg, pFrImg, IntPtr.Zero, 1);
                            
                            //analyse the small video
                            CvInvoke.cvResize(pFrImg, psmallImg,Emgu.CV.CvEnum.INTER.CV_INTER_LINEAR);
                            cur_frame = analyse((int)(p_height*scale),(int)(p_width*scale),psmallImg);
                            if (cur_frame > 0)
                            {
                                if (last_frame < 0 && !is_write)
                                {
                                    //record
                                    str_cur_frame = calc_time(cur_frame,  g_inteval);
                                    listBox1.Items.Add(str_cur_frame + " - ");
                                    writer.Write(str_cur_frame + " - ");
                                    is_write = true;
                                }
                                
                            }
                            else
                            {
                                if (last_frame > 0 && is_write)
                                {
                                    //record
                                    str_last_frame=calc_time(last_frame,  g_inteval);
                                    if (str_last_frame != str_cur_frame)
                                    {
                                        listBox1.Items.RemoveAt(listBox1.Items.Count - 1);
                                        listBox1.Items.Add(str_cur_frame+" - " + str_last_frame);
                                        writer.Write(calc_time(last_frame,  g_inteval) + '\n');
                                        is_write = false;
                                    }
                                    
                                }
                            }
                            last_frame = cur_frame;
                            CvInvoke.cvCopy(pFrame, image_frame, IntPtr.Zero);
                            CvInvoke.cvCopy(pBkImg, image_bk, IntPtr.Zero);
                            CvInvoke.cvCopy(pFrImg, image_fr, IntPtr.Zero);
                            pictureBox1.Image = image_frame.Bitmap;
                            pictureBox2.Image = image_bk.Bitmap;
                            pictureBox3.Image = image_fr.Bitmap;
                            System.Windows.Forms.Application.DoEvents();
                        }//end else
                        
                    }//end while
                    
                }//end using
            }//end using
            
            
            
        }
        
        private void combo_precision_SelectedIndexChanged(object sender, EventArgs e)
        {
            
        }
        
        private void listBox1_DoubleClick(object sender, EventArgs e)
        {
            int index = listBox1.SelectedIndex;
            if (index < 0) return;
            string str_time = (string)listBox1.Items[index];
            string[] values = str_time.Split(':',' ');
            if (values.Length >= 3)
            {
                int hour = int.Parse(values[0]);
                int min = int.Parse(values[1]);
                int second = int.Parse(values[2]);
                int tick = (int)((hour * 3600 + min * 60 + second-1));
                if (tick < 0)
                    tick = 0;
                tick = (int)(tick * fps);
                nFrmNum = tick/g_inteval;
                CvInvoke.cvSetCaptureProperty(pCapture, Emgu.CV.CvEnum.CAP_PROP.CV_CAP_PROP_POS_FRAMES,tick);
                status = Status.Running;//重新读取视频
            }
        }
        
        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            status = Status.Break; ;
        }
        
        
        
    }
}
