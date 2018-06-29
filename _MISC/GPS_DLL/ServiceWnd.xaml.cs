using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Collections;
using System.IO;
using Gavaghan.Geodesy;
using IniParser;
using System.Threading;
using CWP;
using GeodesyLib;


namespace SerialPortTerminal
{
    public struct GeoPosInfo
    {
        public GlobalCoordinates GeoPos;
        public String End;
    }

    /// <summary>
    /// Interaction logic for ServiceWnd.xaml
    /// </summary>
    public partial class ServiceWnd : Window
    {        
       
        public bool m_bShow = false;
        int GUIelems = 0;
        int iThickness = 5;

        private ArrayList geoMapPosList = new ArrayList();
        public GeoMapController geoMapCtrlServiceWnd;

        Point? lastCenterPositionOnTarget;
        Point? lastMousePositionOnTarget;
        Point? lastDragPoint;

        public ServiceWnd()
        {
            InitializeComponent();
            this.Width = 590;
            this.Height = 500;


            scrollViewer.ScrollChanged += OnScrollViewerScrollChanged;
            scrollViewer.MouseLeftButtonUp += OnMouseLeftButtonUp;
            scrollViewer.PreviewMouseLeftButtonUp += OnMouseLeftButtonUp;
            scrollViewer.PreviewMouseWheel += OnPreviewMouseWheel;

            scrollViewer.PreviewMouseLeftButtonDown += OnMouseLeftButtonDown;
            scrollViewer.MouseMove += OnMouseMove;

            //slider.ValueChanged += OnSliderValueChanged;

          
            #region LOAD coastmap data

            String path = Directory.GetCurrentDirectory() + "\\koreanmap.txt";
            string[] lines = File.ReadAllLines(@path);
            int i = 0;

            foreach (string ln in lines)
            {
                if (ln != "")
                {
                    GeoPosInfo gpNew = new GeoPosInfo();
                    if(i==3 || i==1613 || i==1646 || i==2044 || i==2053 || i==2430 || i==2831 || i==3240 || i==3937 || i==3997 || i==4125 || i==4174 || i==4268 || i==4301 || i==4326)
                        gpNew.End = "E";
                    else 
                        gpNew.End = " ";
                    String[] tmpStr = ln.Split(' ');

                    gpNew.GeoPos.Latitude = new Angle(Convert.ToDouble(tmpStr[0]));
                    gpNew.GeoPos.Longitude = new Angle(Convert.ToDouble(tmpStr[1]));

                    geoMapPosList.Add(gpNew);
                    i++;
                }
            }

            #endregion

            #region INIT Geo parameters

            String centerLat = "33.5112"; //mainWnd.servWndIni["GEODETIC_POINTS"]["GeoCenterPosLat"];
            String centerLon = "126.4927"; //mainWnd.servWndIni["GEODETIC_POINTS"]["GeoCenterPosLon"];
            String rotAngle = "31.1"; //mainWnd.servWndIni["GEODETIC_POINTS"]["GeoRotAngle"];
            String viewPtX = "2047.8533024783"; //mainWnd.servWndIni["GEODETIC_POINTS"]["ViewPointX"];
            String viewPtY = "2054.55483884003"; //mainWnd.servWndIni["GEODETIC_POINTS"]["ViewPointY"];
            String bottomRightLat = "33.271111"; //mainWnd.servWndIni["GEODETIC_POINTS"]["GeoPosBottomRightLat"];
            String bottomRightLon = "126.050277"; //mainWnd.servWndIni["GEODETIC_POINTS"]["GeoPosBottomRightLon"];
            String bottomViewPtX = "1962.27533271396"; //mainWnd.servWndIni["GEODETIC_POINTS"]["ViewPointBottomRightX"];
            String bottomViewPtY = "2107.57728335441"; // mainWnd.servWndIni["GEODETIC_POINTS"]["ViewPointBottomRightY"];

            geoMapCtrlServiceWnd = new GeoMapController();
            GlobalCoordinates sCenter = new GlobalCoordinates(Convert.ToDouble(centerLat), Convert.ToDouble(centerLon));
            geoMapCtrlServiceWnd.SetCenterPosition(sCenter);
            geoMapCtrlServiceWnd.SetRotationDegree(Convert.ToDouble(rotAngle));

            Point viewPt = new Point(Convert.ToDouble(viewPtX), Convert.ToDouble(viewPtY));
            geoMapCtrlServiceWnd.SetViewPoint(viewPt);
            geoMapCtrlServiceWnd.SetMeterPerPixel(new GlobalCoordinates(Convert.ToDouble(bottomRightLat), Convert.ToDouble(bottomRightLon)),
                                                  new Point(Convert.ToDouble(bottomViewPtX), Convert.ToDouble(bottomViewPtY)));

            #endregion

            InitMap();

        }
       
        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            this.Visibility = Visibility.Hidden;
            e.Cancel = true;
            m_bShow = false;

        }
        private static Action EmptyDelegate = delegate() { };

        public void InitMap()
        {   
        #region Draw a map

            Line[] lnGeometry = new Line[geoMapPosList.Count];

            for (int i = 0; i < geoMapPosList.Count; i++ )
            {
                if (i == geoMapPosList.Count - 1)
                    break;

                GeoPosInfo geoPt1 = (GeoPosInfo)geoMapPosList[i];

                if (geoPt1.End == "E")
                {
                    i++;
                    if (i == geoMapPosList.Count - 1)
                        break;
                    geoPt1 = (GeoPosInfo)geoMapPosList[i];
                }

                if (lnGeometry[i] == null)
                    lnGeometry[i] = new Line();
                lnGeometry[i].Stroke = System.Windows.Media.Brushes.Black;

                Point pt = geoMapCtrlServiceWnd.GetWindowPos(geoPt1.GeoPos);
                //pt = geoMapCtrlServiceWnd.Rotate(pt, geoMapCtrlServiceWnd.rotAngle);

                lnGeometry[i].X1 = pt.X;
                lnGeometry[i].Y1 = pt.Y;

                lnGeometry[i].HorizontalAlignment = HorizontalAlignment.Left;
                lnGeometry[i].VerticalAlignment = VerticalAlignment.Center;
                lnGeometry[i].StrokeThickness = 2;

                GeoPosInfo geoPt2 = (GeoPosInfo)geoMapPosList[i + 1];

                Point pt2 = geoMapCtrlServiceWnd.GetWindowPos(geoPt2.GeoPos);
                //pt2 = geoMapCtrlServiceWnd.Rotate(pt2, geoMapCtrlServiceWnd.rotAngle);

                lnGeometry[i].X2 = pt2.X;
                lnGeometry[i].Y2 = pt2.Y;
        
                ServiceWndMainCanvas.Children.Add(lnGeometry[i]);
            }
        #endregion   
           
            GUIelems = ServiceWndMainCanvas.Children.Count;
        }
   
        public void DrawTarget(double nLat, double nLon)
        {
            int loopCounter = ServiceWndMainCanvas.Children.Count - GUIelems;

            if (loopCounter != 0)
            {
                for (int i = 0; i < loopCounter; i++)
                {
                    ServiceWndMainCanvas.Children.Remove(ServiceWndMainCanvas.Children[GUIelems]);
                }
            }
           
            if (nLat == 0 && nLon == 0)
                return;

            // get the window position point
            GeoPosInfo gpsPos = new GeoPosInfo();
            gpsPos.GeoPos.Latitude = new Angle(nLat);
            gpsPos.GeoPos.Longitude = new Angle(nLon);

            Point pt = geoMapCtrlServiceWnd.GetWindowPos(gpsPos.GeoPos);

            iThickness += 5;
            if (iThickness == 20)
                iThickness = 5;

            Ellipse ellipse = new Ellipse();

            SolidColorBrush mySolidColorBrush = new SolidColorBrush();
            mySolidColorBrush.Color =  Colors.Red;

            ellipse.Stroke = mySolidColorBrush;
            ellipse.StrokeThickness = iThickness;
            ellipse.Width = iThickness + 40;
            ellipse.Height = iThickness + 40;

            ServiceWndMainCanvas.Children.Add(ellipse);
            Canvas.SetLeft(ellipse, pt.X - ellipse.Width / 2);
            Canvas.SetTop(ellipse, pt.Y - ellipse.Height / 2);
        }

        void OnMouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            scrollViewer.Cursor = Cursors.Arrow;
            scrollViewer.ReleaseMouseCapture();
            lastDragPoint = null;
        }

        void OnSliderValueChanged(object sender,
             RoutedPropertyChangedEventArgs<double> e)
        {
            scaleTransform.ScaleX = 0.5; // e.NewValue;
            scaleTransform.ScaleY = 0.5; // e.NewValue;

            var centerOfViewport = new Point(scrollViewer.ViewportWidth / 2,
                                             scrollViewer.ViewportHeight / 2);
            lastCenterPositionOnTarget = scrollViewer.TranslatePoint(centerOfViewport, grid);
        }
        void OnMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            var mousePos = e.GetPosition(scrollViewer);
            if (mousePos.X <= scrollViewer.ViewportWidth && mousePos.Y <
                scrollViewer.ViewportHeight) //make sure we still can use the scrollbars
            {
                scrollViewer.Cursor = Cursors.SizeAll;
                lastDragPoint = mousePos;
                Mouse.Capture(scrollViewer);
            }
        }

        void OnPreviewMouseWheel(object sender, MouseWheelEventArgs e)
        {
            lastMousePositionOnTarget = Mouse.GetPosition(grid);

            if (e.Delta > 0)
            {
                slider.Value += 1;
            }
            if (e.Delta < 0)
            {
                slider.Value -= 1;
            }

            e.Handled = true;
        }
        void OnMouseMove(object sender, MouseEventArgs e)
        {
            if (lastDragPoint.HasValue)
            {
                Point posNow = e.GetPosition(scrollViewer);

                double dX = posNow.X - lastDragPoint.Value.X;
                double dY = posNow.Y - lastDragPoint.Value.Y;

                lastDragPoint = posNow;

                scrollViewer.ScrollToHorizontalOffset(scrollViewer.HorizontalOffset - dX);
                scrollViewer.ScrollToVerticalOffset(scrollViewer.VerticalOffset - dY);
            }
        }

        void OnScrollViewerScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            if (e.ExtentHeightChange != 0 || e.ExtentWidthChange != 0)
            {
                Point? targetBefore = null;
                Point? targetNow = null;

                if (!lastMousePositionOnTarget.HasValue)
                {
                    if (lastCenterPositionOnTarget.HasValue)
                    {
                        var centerOfViewport = new Point(scrollViewer.ViewportWidth / 2,
                                                         scrollViewer.ViewportHeight / 2);
                        Point centerOfTargetNow =
                              scrollViewer.TranslatePoint(centerOfViewport, grid);

                        targetBefore = lastCenterPositionOnTarget;
                        targetNow = centerOfTargetNow;
                    }
                }
                else
                {
                    targetBefore = lastMousePositionOnTarget;
                    targetNow = Mouse.GetPosition(grid);

                    lastMousePositionOnTarget = null;
                }

                if (targetBefore.HasValue)
                {
                    double dXInTargetPixels = targetNow.Value.X - targetBefore.Value.X;
                    double dYInTargetPixels = targetNow.Value.Y - targetBefore.Value.Y;

                    double multiplicatorX = e.ExtentWidth / grid.Width;
                    double multiplicatorY = e.ExtentHeight / grid.Height;

                    double newOffsetX = scrollViewer.HorizontalOffset -
                                        dXInTargetPixels * multiplicatorX;
                    double newOffsetY = scrollViewer.VerticalOffset -
                                        dYInTargetPixels * multiplicatorY;

                    if (double.IsNaN(newOffsetX) || double.IsNaN(newOffsetY))
                    {
                        return;
                    }

                    scrollViewer.ScrollToHorizontalOffset(newOffsetX);
                    scrollViewer.ScrollToVerticalOffset(newOffsetY);
                }
            }
        }

    }
}