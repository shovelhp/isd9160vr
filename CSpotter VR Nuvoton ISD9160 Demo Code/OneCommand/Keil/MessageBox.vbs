Set objArgs = WScript.Arguments 
messageText = "Build audio resource faild."  +  vbCrLf + _
"Check the output window of keil to see error message."  +  vbCrLf + _
"Double click the error message and can navigate to the error line." 
MsgBox messageText, vbCritical, "Audio Resource Building Error"  
