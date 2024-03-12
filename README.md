# ExtremeInjector

## Intruction

ExtremeInjector is a .NET program DLL injector which is runing on Windows x64 system. Hope it useful for you :) 


![](MainWindow.png)



## Directory Structure
![](Dir.png)


## Usage

Put Hook.ini in your inject target process'm path since the ProxyLoader will read the ini to load the .NET DLL.

```
[HookInfo]
Dll=C:\Users\Test\Desktop\HookDemo64.dll
TypeName=HookDemo.Class1
Method=TestMethod
Argument=You are right!

```

The DLL is a .NET DLL that you want to inject.

The other parameters are used for `ExecuteInDefaultAppDomain` API.

```c++
HRESULT ExecuteInDefaultAppDomain (  
    [in] LPCWSTR pwzAssemblyPath,  
    [in] LPCWSTR pwzTypeName,
    [in] LPCWSTR pwzMethodName,  
    [in] LPCWSTR pwzArgument,  
    [out] DWORD *pReturnValue  
);
```


Demo Code:
```csharp
namespace HookDemo
{
    public class Class1
    {
        public static int TestMethod(string param)
        {
            Debug.WriteLine("Hello .NET World! " + param);
            return 0;
        }
    }
}

```

![](Demo.png)


## Credits

- Process Explorer X by [Process Explorer X](https://github.com/zodiacon/ProcExpX)