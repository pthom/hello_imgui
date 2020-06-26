This code will be placed in the appropriate folder.

You can customize your application id via 
* ${apkCMake_applicationIdUrlPart}
* ${apkCMake_applicationIdNamePart}
* ${apkCMake_className}

For example:
* apkCMake_applicationIdUrlPart=com.my_company
* apkCMake_applicationIdNamePart=my_app
* apkCMake_className=MyApp

then a file MyApp.java will be created into 

app/src/main/java/com/my_company/my_app/MyApp.java
