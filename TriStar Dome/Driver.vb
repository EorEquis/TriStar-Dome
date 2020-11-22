'tabs=4
' --------------------------------------------------------------------------------
'
' ASCOM Dome driver for TriStarObservatory
'
' Description:	ASCOM Dome Driver for TriStar Observatory.  Implemented to control ROR only
'
' Implements:	ASCOM Dome interface version: 1.0
' Author:		(Eor) EorEquis@tristarobservatory.space
'
' Edit Log:
'
' Date			Who	Vers	Description
' -----------	---	-----	-------------------------------------------------------
' 04-DEC-2017	Eor	0.1.0	Initial edit, from Dome template
' 05-DEC-2017   Eor 0.2.0   WTF is going on with my IDEs?
' 05-DEC-2017   Eor 1.0.0   Initial operating version, tested in SGP
' 07-DEC-2017   Eor 1.1.0   Cleanup, remove LCD
' 07-NOV-2020   Eor 1.2.0   Change to ShutterStatus, Setup Dialog
' ---------------------------------------------------------------------------------
'
'
' Your driver's ID is ASCOM.TriStarObservatory.Dome
'
' The Guid attribute sets the CLSID for ASCOM.DeviceName.Dome
' The ClassInterface/None addribute prevents an empty interface called
' _Dome from being created and used as the [default] interface
'

' This definition is used to select code that's only applicable for one device type
#Const Device = "Dome"

Imports ASCOM
Imports ASCOM.Astrometry
Imports ASCOM.Astrometry.AstroUtils
Imports ASCOM.DeviceInterface
Imports ASCOM.Utilities

Imports System
Imports System.Collections
Imports System.Collections.Generic
Imports System.Globalization
Imports System.Runtime.InteropServices
Imports System.Text

<Guid("9e342f54-ee65-45e6-a8ba-9cea6bfe817c")> _
<ClassInterface(ClassInterfaceType.None)> _
Public Class Dome

    ' The Guid attribute sets the CLSID for ASCOM.TriStar.Dome
    ' The ClassInterface/None addribute prevents an empty interface called
    ' _TriStar from being created and used as the [default] interface

    ' TODO Replace the not implemented exceptions with code to implement the function or
    ' throw the appropriate ASCOM exception.
    '
    Implements IDomeV2

    '
    ' Driver ID and descriptive string that shows in the Chooser
    '
    Friend Shared driverID As String = "ASCOM.TriStar.Dome"
    Private Shared driverDescription As String = "TriStar Dome"
    Private Shared strDriverVersion As String = "3.0.0b"

    Friend Shared comPortProfileName As String = "COM Port" 'Constants used for Profile persistence
    Friend Shared traceStateProfileName As String = "Trace Level"
    Friend Shared comPortDefault As String = "COM1"
    Friend Shared traceStateDefault As String = "False"


    Friend Shared comPort As String ' Variables to hold the currrent device configuration
    Friend Shadows portNum As String
    Friend Shared traceState As Boolean

    Private connectedState As Boolean ' Private variable to hold the connected state
    Private utilities As Util ' Private variable to hold an ASCOM Utilities object
    Private astroUtilities As AstroUtils ' Private variable to hold an AstroUtils object to provide the Range method
    Private TL As TraceLogger ' Private variable to hold the trace logger object (creates a diagnostic log file with information that you specify)
    Private objSerial As ASCOM.Utilities.Serial
    Private statusTimer As System.Timers.Timer

    '
    ' Constructor - Must be public for COM registration!
    '
    Public Sub New()

        ReadProfile() ' Read device configuration from the ASCOM Profile store
        TL = New TraceLogger("", "TriStar")
        TL.Enabled = traceState
        TL.LogMessage("Dome", "Starting initialisation")

        connectedState = False ' Initialise connected to false
        utilities = New Util() ' Initialise util object
        astroUtilities = New AstroUtils 'Initialise new astro utiliites object

        'TODO: Implement your additional construction here
        statusTimer = New Timers.Timer
        statusTimer.Interval = 1000
        statusTimer.AutoReset = True
        AddHandler statusTimer.Elapsed, AddressOf Timer_Tick

        TL.LogMessage("Dome", "Completed initialisation")
    End Sub

    '
    ' PUBLIC COM INTERFACE IDomeV2 IMPLEMENTATION
    '

#Region "Common properties and methods"
    ''' <summary>
    ''' Displays the Setup Dialog form.
    ''' If the user clicks the OK button to dismiss the form, then
    ''' the new settings are saved, otherwise the old values are reloaded.
    ''' THIS IS THE ONLY PLACE WHERE SHOWING USER INTERFACE IS ALLOWED!
    ''' </summary>
    Public Sub SetupDialog() Implements IDomeV2.SetupDialog
        ' consider only showing the setup dialog if not connected
        ' or call a different dialog if connected
        If IsConnected Then
            System.Windows.Forms.MessageBox.Show("Already connected, just press OK")
        End If

        Using F As SetupDialogForm = New SetupDialogForm()
            Dim result As System.Windows.Forms.DialogResult = F.ShowDialog()
            If result = DialogResult.OK Then
                WriteProfile() ' Persist device configuration values to the ASCOM Profile store
            End If
        End Using
    End Sub

    Public ReadOnly Property SupportedActions() As ArrayList Implements IDomeV2.SupportedActions
        Get
            TL.LogMessage("SupportedActions Get", "Returning empty arraylist")
            Return New ArrayList()
        End Get
    End Property

    Public Function Action(ByVal ActionName As String, ByVal ActionParameters As String) As String Implements IDomeV2.Action
        Throw New ActionNotImplementedException("Action " & ActionName & " is not supported by this driver")
    End Function

    Public Sub CommandBlind(ByVal Command As String, Optional ByVal Raw As Boolean = False) Implements IDomeV2.CommandBlind
        CheckConnected("CommandBlind")
        ' Call CommandString and return as soon as it finishes
        Me.CommandString(Command, Raw)
    End Sub

    Public Function CommandBool(ByVal Command As String, Optional ByVal Raw As Boolean = False) As Boolean _
        Implements IDomeV2.CommandBool
        CheckConnected("CommandBool")
        Dim ret As String = CommandString(Command, Raw)
    End Function

    Public Function CommandString(ByVal Command As String, Optional ByVal Raw As Boolean = False) As String _
        Implements IDomeV2.CommandString
        CheckConnected("CommandString")
        ' it's a good idea to put all the low level communication with the device here,
        ' then all communication calls this function
        ' you need something to ensure that only one command is in progress at a time
        ' TODO : Multithreading
        Try
            Dim response As String
            ' CheckConnected("CommandString")
            objSerial.Transmit(Command)
            response = objSerial.ReceiveTerminated("#")
            response = response.Replace("#", "")
            Return response
        Catch ex As Exception
            Return "255"
        End Try
    End Function

    Public Property Connected() As Boolean Implements IDomeV2.Connected
        Get
            TL.LogMessage("Connected Get", IsConnected.ToString())
            Return IsConnected
        End Get
        Set(value As Boolean)
            TL.LogMessage("Connected Set", value.ToString())
            If value = IsConnected Then
                Return
            End If

            If value Then
                TL.LogMessage("Connected Set", "Connecting to port " + comPort)
                portNum = Right(comPort, Len(comPort) - 3)
                objSerial = New ASCOM.Utilities.Serial
                objSerial.Port = CInt(portNum)
                objSerial.Speed = SerialSpeed.ps38400
                objSerial.Connected = True
                statusTimer.Enabled = True
                wait(500)      ' Give the arduino a moment to get connected.
                connectedState = True
            Else
                TL.LogMessage("Connected Set", "Disconnecting from port " + comPort)
                objSerial.Connected = False
                objSerial = Nothing
                statusTimer.Enabled = False
                connectedState = False
            End If
        End Set
    End Property

    Public ReadOnly Property Description As String Implements IDomeV2.Description
        Get
            ' this pattern seems to be needed to allow a public property to return a private field
            Dim d As String = driverDescription
            TL.LogMessage("Description Get", d)
            Return d
        End Get
    End Property

    Public ReadOnly Property DriverInfo As String Implements IDomeV2.DriverInfo
        Get
            Dim m_version As Version = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version
            ' TODO customise this driver description
            Dim s_driverInfo As String = "Information about the driver itself. Version: " + strDriverVersion
            TL.LogMessage("DriverInfo Get", s_driverInfo)
            Return s_driverInfo
        End Get
    End Property

    Public ReadOnly Property DriverVersion() As String Implements IDomeV2.DriverVersion
        Get
            ' Get our own assembly and report its version number
            TL.LogMessage("DriverVersion Get", strDriverVersion)
            Return strDriverVersion
        End Get
    End Property

    Public ReadOnly Property InterfaceVersion() As Short Implements IDomeV2.InterfaceVersion
        Get
            TL.LogMessage("InterfaceVersion Get", "2")
            Return 2
        End Get
    End Property

    Public ReadOnly Property Name As String Implements IDomeV2.Name
        Get
            Dim s_name As String = "TriStar Dome"
            TL.LogMessage("Name Get", s_name)
            Return s_name
        End Get
    End Property

    Public Sub Dispose() Implements IDomeV2.Dispose
        ' Clean up the tracelogger and util objects
        TL.Enabled = False
        TL.Dispose()
        TL = Nothing
        utilities.Dispose()
        utilities = Nothing
        astroUtilities.Dispose()
        astroUtilities = Nothing
    End Sub

#End Region

#Region "IDome Implementation"

    Private domeShutterState As Integer = 1 ' Variable to hold the status of the shutter

    Public Sub AbortSlew() Implements IDomeV2.AbortSlew
        CommandBlind("abrt#")
        TL.LogMessage("AbortSlew", "Completed")
    End Sub

    Public ReadOnly Property Altitude() As Double Implements IDomeV2.Altitude
        Get
            TL.LogMessage("Altitude Get", "Not implemented")
            Throw New ASCOM.PropertyNotImplementedException("Altitude", False)
        End Get
    End Property

    Public ReadOnly Property AtHome() As Boolean Implements IDomeV2.AtHome
        Get
            TL.LogMessage("AtHome", "Not implemented")
            Throw New ASCOM.PropertyNotImplementedException("AtHome", False)
        End Get
    End Property

    Public ReadOnly Property AtPark() As Boolean Implements IDomeV2.AtPark
        Get
            TL.LogMessage("AtPark", "Not implemented")
            Throw New ASCOM.PropertyNotImplementedException("AtPark", False)
        End Get
    End Property

    Public ReadOnly Property Azimuth() As Double Implements IDomeV2.Azimuth
        Get
            TL.LogMessage("Azimuth", "Not implemented")
            Throw New ASCOM.PropertyNotImplementedException("Azimuth", False)
        End Get
    End Property

    Public ReadOnly Property CanFindHome() As Boolean Implements IDomeV2.CanFindHome
        Get
            TL.LogMessage("CanFindHome Get", False.ToString())
            Return False
        End Get
    End Property

    Public ReadOnly Property CanPark() As Boolean Implements IDomeV2.CanPark
        Get
            TL.LogMessage("CanPark Get", False.ToString())
            Return False
        End Get
    End Property

    Public ReadOnly Property CanSetAltitude() As Boolean Implements IDomeV2.CanSetAltitude
        Get
            TL.LogMessage("CanSetAltitude Get", False.ToString())
            Return False
        End Get
    End Property

    Public ReadOnly Property CanSetAzimuth() As Boolean Implements IDomeV2.CanSetAzimuth
        Get
            TL.LogMessage("CanSetAzimuth Get", False.ToString())
            Return False
        End Get
    End Property

    Public ReadOnly Property CanSetPark() As Boolean Implements IDomeV2.CanSetPark
        Get
            TL.LogMessage("CanSetPark Get", False.ToString())
            Return False
        End Get
    End Property

    Public ReadOnly Property CanSetShutter() As Boolean Implements IDomeV2.CanSetShutter
        Get
            TL.LogMessage("CanSetShutter Get", True.ToString())
            Return True
        End Get
    End Property

    Public ReadOnly Property CanSlave() As Boolean Implements IDomeV2.CanSlave
        Get
            TL.LogMessage("CanSlave Get", False.ToString())
            Return False
        End Get
    End Property

    Public ReadOnly Property CanSyncAzimuth() As Boolean Implements IDomeV2.CanSyncAzimuth
        Get
            TL.LogMessage("CanSyncAzimuth Get", False.ToString())
            Return False
        End Get
    End Property

    Public Sub CloseShutter() Implements IDomeV2.CloseShutter
        domeShutterState = CInt(CommandString("clos#"))
        If domeShutterState = 3 Then
            ' If domeShutterState = ShutterState.shutterClosing Then
            TL.LogMessage("CloseShutter", "Shutter has been closed")
        Else
            TL.LogMessage("CloseShutter", "Error closing shutter")
        End If
    End Sub

    Public Sub FindHome() Implements IDomeV2.FindHome
        TL.LogMessage("FindHome", "Not implemented")
        Throw New ASCOM.MethodNotImplementedException("FindHome")
    End Sub

    Public Sub OpenShutter() Implements IDomeV2.OpenShutter
        domeShutterState = CInt(CommandString("open#"))
        If domeShutterState = 2 Then
            ' If domeShutterState = ShutterState.shutterOpening Then
            TL.LogMessage("OpenShutter", "Shutter has been opened")
        Else
            TL.LogMessage("OpenShutter", "Error opening shutter")
        End If
    End Sub

    Public Sub Park() Implements IDomeV2.Park
        TL.LogMessage("Park", "Not implemented")
        Throw New ASCOM.MethodNotImplementedException("Park")
    End Sub

    Public Sub SetPark() Implements IDomeV2.SetPark
        TL.LogMessage("SetPark", "Not implemented")
        Throw New ASCOM.MethodNotImplementedException("SetPark")
    End Sub

    Public ReadOnly Property ShutterStatus() As ShutterState Implements IDomeV2.ShutterStatus
        Get
            'Select Case domeShutterState
            '    Case 0
            '        TL.LogMessage("ShutterStatus", ShutterState.shutterOpen.ToString())
            '        Return ShutterState.shutterOpen
            '    Case 1
            '        TL.LogMessage("ShutterStatus", ShutterState.shutterClosed.ToString())
            '        Return ShutterState.shutterClosed
            '    Case 2
            '        TL.LogMessage("ShutterStatus", ShutterState.shutterOpening.ToString())
            '        Return ShutterState.shutterOpening
            '    Case 3
            '        TL.LogMessage("ShutterStatus", ShutterState.shutterClosing.ToString())
            '        Return ShutterState.shutterClosing
            '    Case Else
            '        TL.LogMessage("ShutterStatus", ShutterState.shutterError.ToString())
            '        Return ShutterState.shutterError
            'End Select

            Select Case domeShutterState
                Case 0
                    TL.LogMessage("ShutterStatus", ShutterState.shutterOpen.ToString())
                    Return ShutterState.shutterOpen
                Case 1
                    TL.LogMessage("ShutterStatus", ShutterState.shutterClosed.ToString())
                    Return ShutterState.shutterClosed
                Case 2
                    TL.LogMessage("ShutterStatus", ShutterState.shutterOpening.ToString())
                    Return ShutterState.shutterOpening
                Case 3
                    TL.LogMessage("ShutterStatus", ShutterState.shutterClosing.ToString())
                    Return ShutterState.shutterClosing
                Case 4
                    TL.LogMessage("ShutterStatus", ShutterState.shutterError.ToString())
                    Return ShutterState.shutterError
                Case Else
                    TL.LogMessage("ShutterStatus", "Communication error, domeShutterstate = " + domeShutterState.ToString)
                    Return ShutterState.shutterError
            End Select
        End Get
    End Property

    Public Property Slaved() As Boolean Implements IDomeV2.Slaved
        Get
            TL.LogMessage("Slaved Get", False.ToString())
            Return False
        End Get
        Set(value As Boolean)
            TL.LogMessage("Slaved Set", "not implemented")
            Throw New ASCOM.PropertyNotImplementedException("Slaved", True)
        End Set
    End Property

    Public Sub SlewToAltitude(Altitude As Double) Implements IDomeV2.SlewToAltitude
        TL.LogMessage("SlewToAltitude", "Not implemented")
        Throw New ASCOM.MethodNotImplementedException("SlewToAltitude")
    End Sub

    Public Sub SlewToAzimuth(Azimuth As Double) Implements IDomeV2.SlewToAzimuth
        TL.LogMessage("SlewToAzimuth", "Not implemented")
        Throw New ASCOM.MethodNotImplementedException("SlewToAzimuth")
    End Sub

    Public ReadOnly Property Slewing() As Boolean Implements IDomeV2.Slewing
        Get
            TL.LogMessage("Slewing Get", False.ToString())
            Return False
        End Get
    End Property

    Public Sub SyncToAzimuth(Azimuth As Double) Implements IDomeV2.SyncToAzimuth
        TL.LogMessage("SyncToAzimuth", "Not implemented")
        Throw New ASCOM.MethodNotImplementedException("SyncToAzimuth")
    End Sub

#End Region

#Region "Private properties and methods"
    ' here are some useful properties and methods that can be used as required
    ' to help with

#Region "ASCOM Registration"

    Private Shared Sub RegUnregASCOM(ByVal bRegister As Boolean)

        Using P As New Profile() With {.DeviceType = "Dome"}
            If bRegister Then
                P.Register(driverID, driverDescription)
            Else
                P.Unregister(driverID)
            End If
        End Using

    End Sub

    <ComRegisterFunction()> _
    Public Shared Sub RegisterASCOM(ByVal T As Type)

        RegUnregASCOM(True)

    End Sub

    <ComUnregisterFunction()> _
    Public Shared Sub UnregisterASCOM(ByVal T As Type)

        RegUnregASCOM(False)

    End Sub

#End Region

    ''' <summary>
    ''' Returns true if there is a valid connection to the driver hardware
    ''' </summary>
    Private ReadOnly Property IsConnected As Boolean
        Get
            ' TODO check that the driver hardware connection exists and is connected to the hardware
            Return connectedState
        End Get
    End Property

    ''' <summary>
    ''' Use this function to throw an exception if we aren't connected to the hardware
    ''' </summary>
    ''' <param name="message"></param>
    Private Sub CheckConnected(ByVal message As String)
        If Not IsConnected Then
            Throw New NotConnectedException(message)
        End If
    End Sub

    ''' <summary>
    ''' Read the device configuration from the ASCOM Profile store
    ''' </summary>
    Friend Sub ReadProfile()
        Using driverProfile As New Profile()
            driverProfile.DeviceType = "Dome"
            traceState = Convert.ToBoolean(driverProfile.GetValue(driverID, traceStateProfileName, String.Empty, traceStateDefault))
            comPort = driverProfile.GetValue(driverID, comPortProfileName, String.Empty, comPortDefault)
        End Using
    End Sub

    ''' <summary>
    ''' Write the device configuration to the  ASCOM  Profile store
    ''' </summary>
    Friend Sub WriteProfile()
        Using driverProfile As New Profile()
            driverProfile.DeviceType = "Dome"
            driverProfile.WriteValue(driverID, traceStateProfileName, traceState.ToString())
            driverProfile.WriteValue(driverID, comPortProfileName, comPort.ToString())
        End Using

    End Sub

#End Region

#Region "My Functions and methods"
    Private Function getShutterState() As ShutterState
        Return CommandString("info#")
    End Function

    Private Sub Timer_Tick(source As Object, e As EventArgs)
        domeShutterState = CInt(getShutterState())
    End Sub

    Private Sub wait(ByVal interval As Integer)
        ' Delays interval milliseconds, without blocking the UI
        Dim sw As New Stopwatch
        sw.Start()
        Do While sw.ElapsedMilliseconds < interval
            Application.DoEvents()
        Loop
        sw.Stop()
    End Sub

#End Region

End Class
