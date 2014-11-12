﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO.Ports;

namespace Telemetry
{
    class Serial
    {
        public static byte CHANGEMODE = 0x00;
        public static byte FORWARD = 0x01;
        public static byte BACK = 0x02;
        public static byte LEFT = 0x03;
        public static byte RIGHT = 0x04;
        public static byte STOP = 0x05;

        private SerialPort connection;

        private System.Text.StringBuilder log = new System.Text.StringBuilder();
        private int counter;

        public Serial(String port, Int32 baud, Parity p, Int32 databits, StopBits b)
        {
            connection = new SerialPort(port, baud, p, databits, b);
            connection.DataReceived += new SerialDataReceivedEventHandler(_serialPort_DataReceived); 
            if (!connection.IsOpen)
            {
                connection.Open();
            }
            connection.Handshake = System.IO.Ports.Handshake.None;
            connection.DiscardInBuffer();
            connection.DiscardOutBuffer();

            log.Append("Log file for spiderbot.\n");
            log.Append("PORT: " + port + "\n");
            log.Append("BAUDRATE: " + baud.ToString() + "\n");
            log.Append("PARITY: NONE\n");
            log.Append("DATABITS: " + databits.ToString() + "\n");
            log.Append("STOPBITS: 1\n");

            counter = 0;
        } 
 
        void _serialPort_DataReceived(object sender, SerialDataReceivedEventArgs e) 
        {
            //Buffer to store the data
            byte[] buffer = new byte[connection.ReadBufferSize];
            
            //Save how many byter are read
            int bytesRead = connection.Read(buffer, 0, buffer.Length);

            //Append to log stringbuilder 
            for (int i = 0; i < bytesRead; i++)
            {
                log.Append(counter.ToString()+":\t0b"+Convert.ToString(buffer[i], 2).PadLeft(8, '0')+"\t0x"+Convert.ToString(buffer[i], 16).PadLeft(2, '0')+"\n");
                counter++;
            }
        }

        public void sendByte(byte b)
        {
            byte[] data = new byte[]{b};

            connection.Write(data, 0, 1);
        }

        public void writeFile()
        {
            System.IO.File.WriteAllText("Log.txt", log.ToString());
        }
    }

    class Telemetry
    {

        static void Main(string[] args)
        {
            Console.WriteLine("+----------------------------+");
            Console.WriteLine("|Epic Telemetry for Spiderbot|");
            Console.WriteLine("+----------------------------+");
            Console.WriteLine();

            Console.Write("Enter COM port>> ");
            string port = Console.ReadLine();
            Serial serial = new Serial(port, 115200, Parity.None, 8, StopBits.One);

            Console.WriteLine("Connection established.");
            Console.WriteLine();
            Console.WriteLine("+--------------------+");
            Console.WriteLine("| W: FORWARD         |");
            Console.WriteLine("| S: BACK            |");
            Console.WriteLine("| A: LEFT            |");
            Console.WriteLine("| D: RIGHT           |");
            Console.WriteLine("| C: CHANGE MODE     |");
            Console.WriteLine("| H: STOP (HALT)     |");
            Console.WriteLine("| F: SAVE LOG FILE   |");
            Console.WriteLine("| P: EXIT PROGRAM    |");
            Console.WriteLine("+--------------------+");


            Console.Write("Press a key>> ");
            while (true)
            {
                if (Console.KeyAvailable)
                {
                    ConsoleKeyInfo key = Console.ReadKey(true);
                    
                    switch (key.Key)
                    {
                        case ConsoleKey.C:
                            Console.WriteLine("C");
                            serial.sendByte(Serial.CHANGEMODE);
                            break;
                        case ConsoleKey.W:
                            Console.WriteLine("W");
                            serial.sendByte(Serial.FORWARD);
                            break;
                        case ConsoleKey.A:
                            Console.WriteLine("A");
                            serial.sendByte(Serial.LEFT);
                            break;
                        case ConsoleKey.S:
                            Console.WriteLine("S");
                            serial.sendByte(Serial.BACK);
                            break;
                        case ConsoleKey.D:
                            Console.WriteLine("D");
                            serial.sendByte(Serial.RIGHT);
                            break;
                        case ConsoleKey.H:
                            Console.WriteLine("H");
                            serial.sendByte(Serial.STOP);
                            break;
                        case ConsoleKey.F:
                            Console.WriteLine("F");
                            serial.writeFile();
                            break;
                        case ConsoleKey.P:
                            Console.WriteLine("P, Bye bye spiderbot!");
                            System.Threading.Thread.Sleep(750);
                            System.Environment.Exit(0);
                            break;
                        default:
                            Console.WriteLine("Not an active key");
                            break;
                    }

                    Console.Write("Press a key>> ");
                }
            }  
        }
    }
}
