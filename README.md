# Backstab
## _Kill EDR Protected Processes_
Have these local admin credentials but the EDR is standing in the way? Unhooking or direct syscalls are not working against the EDR? Well, why not just kill it? Backstab is a tool capable of killing antimalware protected processes by leveraging sysinternals’ Process Explorer (ProcExp) driver, which is signed by Microsoft. 


#### What can it do?
```
Usage: backstab.exe <-n name || -p PID> [options]  
	-n,		Choose process by name, including the .exe suffix
	-p, 	Choose process by PID
	-l, 	List handles of protected process
	-k, 	Kill the protected process by closing its handles
	-x, 	Close a specific handle
	-d, 	Specify path to where ProcExp will be extracted
	-s, 	Specify service name registry key
	-u, 	Unload ProcExp driver
	-h, 	Print this menu

	Examples:
	backstab.exe -n cyserver.exe -k 			[kill cyserver]
	backstab.exe -n cyserver.exe -x E4C 		[Close handle E4C of cyserver]
	backstab.exe -n cyserver.exe -l 			[list all handles of cyserver]
	tbackstab.exe -p 4326 -k -d c:\\driver.sys 	[kill protected process with PID 4326, extract ProcExp driver to C:\ drive]
```


#### How is that possible?
ProcExp has a signed kernel driver that it loads on startup, which allows it to kill handles that cannot be killed even as an administrator. When you use the UI, you cannot kill a protected process, but you can kill it handles because ProcExp UI instructs the kernel driver to kill those handles. Backstab does the same thing but without the UI element. 

#### OpSec
Here is a quick rundown of what happens
1.	Embedded driver is dropped to disk
2.	Registry key under HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services is created
3.	The privilege SE_PRIVILEGE_ENABLED is acquired because it is necessary to load the driver
4.	Driver is loaded using NtLoadDriver to avoid creating a service
5.	The created Registry key is deleted (service not visible during execution)
6.	Communication with the driver is via using DeviceIoControl
7.	For handle enumeration, NtQuerySystemInformation is called

#### What you should also know
1.	The behavior of the tool mimics that of ProcExp. ProcExp drops the driver to the disk, create registry key under HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services, calls NtLoadDriver, and then delete the registry key
2.	You can specify the location to which the driver is dropped and the service name
3.	When done, the app will unload the driver if you specify the option to unload the driver. The driver is unloaded by first re-creating the registry keys and then calling NtUnloadDriver
4.	The loaded driver is signed by MS
5.	The process does not attempt to directly kill protected processes handles, it instructs ProcExp driver to kill them. You won't be accused of attempting to tamper with any processes


#### Further Research
While the tool purpose is listing and killing handles, the opportunities are vast. It is possible to duplicate the handles to your own process instead of killing them. This could allow for deeper tampering where you write to files, fire events, hold mutexes. To support further research, I tried to make the code readable and split it to many methods to facilitate reuse, I also left a description on all ProcExp related methods. Feel free to reach out to me on [Twitter](https://twitter.com/yas_o_h) or by [Email](mailto:y.o.alhazmi@pm.me)


#### Credits
- Author: Yasser Alhazmi (@Yas_o_h)
- Pavel Yosifovich: [(@Zodiacon)](https://twitter.com/zodiacon) mentioned to us during his awesome [Windows Internals Course](https://scorpiosoftware.net/category/windows-internals/) that kernel drivers like ProcExp might cause too much unintended damage
- Cornelis de Plaa and Outflank Team: for [Ps-Tools](https://github.com/outflanknl/Ps-Tools/blob/master/README.md) and their outstanding Github repos, always informative
- Mark Russinovich: for ProcExp, and all Sysinternals tools!

