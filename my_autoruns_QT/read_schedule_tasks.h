#ifndef READ_SCHEDULE_TASKS_H
#define READ_SCHEDULE_TASKS_H

#include <iostream>
#include <windows.h> //MultiByteToWideChar等
#include <tchar.h>
#include <map>
#include <winreg.h>
#include <Wincrypt.h>
#include <stdlib.h>
#include <Softpub.h>

#include "format_something.h"
#include "read_register.h"

#include <vector>

#include <comdef.h>
#include <taskschd.h>
#include <winreg.h>

using namespace _com_util;

#pragma comment(lib, "version.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Wintrust.lib")
#pragma comment(lib, "taskschd.lib")

//读C:\Windows\System32\Tasks下指定子目录
//引用参数
map<char*, char*> read_schedule_task_subfolder(map<char*, char*> *map_taskpath_imagepath,ITaskFolder *pSubFolder)
{
    //  -------------------------------------------------------
    //  Get the registered tasks in the folder.

    HRESULT hr;
    //已注册的任务集合 pTaskCollection
    IRegisteredTaskCollection* pTaskCollection = NULL;
    //获取任务集合：TASK_ENUM_HIDDEN 可检索文件夹中包含隐藏任务的所有任务
    hr = pSubFolder->GetTasks( TASK_ENUM_HIDDEN, &pTaskCollection );

    //pSubFolder->Release(); //留待递归

    if( FAILED(hr) )
    {
        printf("Cannot get the registered tasks.: %x", hr);
        //CoUninitialize();
        return *map_taskpath_imagepath;
    }

    //任务数量
    LONG numTasks = 0;
    hr = pTaskCollection->get_Count(&numTasks);

    //遍历任务集合pTaskCollection
    for(LONG i=0; i < numTasks; i++)
    {
        //当前遍历到的任务 pRegisteredTask
        IRegisteredTask* pRegisteredTask = NULL;
        hr = pTaskCollection->get_Item( _variant_t(i+1), &pRegisteredTask );

        //成功遍历到任务 pRegisteredTask
        if( SUCCEEDED(hr) )
        {
            //任务路径
            BSTR bstrTaskPath = NULL;
            //任务的所有组件，例如任务设置、触发器、操作和注册信息
            ITaskDefinition* taskdef = NULL;
            //任务执行的操作
            IActionCollection* ppact = NULL;
            //所有操作对象继承的公共属性
            IAction* pact = NULL;
            //执行命令行操作
            IExecAction* peact = NULL;
            //
            BSTR bstrTaskimg = NULL;

            BSTR taskName = NULL;
            hr = pRegisteredTask->get_Name(&taskName);


            //成功取文件名
            if( SUCCEEDED(hr) )
            {
                SysFreeString(taskName);
                //文件相对根目录的路径
                pRegisteredTask->get_Path(&bstrTaskPath);
                //任务定义
                pRegisteredTask->get_Definition(&taskdef);
                //从任务组件里获取任务执行的操作集合
                taskdef->get_Actions(&ppact);
                //从任务操作集合里获取指定操作到pact
                ppact->get_Item(1, &pact);
                //获取执行的imagepath
                pact->QueryInterface(IID_IExecAction, (void**)&peact);

                if (peact != NULL)
                {
                    //获取可执行文件的路径->char*
                    hr = peact->get_Path(&bstrTaskimg);
                    char* task_imagepath = ConvertBSTRToString(bstrTaskimg);
                    QString task_imagepath_qstr = charstr_to_QString(task_imagepath);
                    //路径标准化
                    format_imagepath(&task_imagepath_qstr);

                    char* task_path = ConvertBSTRToString(bstrTaskPath);

                    if (SUCCEEDED(hr))
                    {
                        // imagepath非空
                        if (task_imagepath != "")
                        {
                            //存入map一项
                            //map<char*,char*>
                            (*map_taskpath_imagepath)[task_path] = task_imagepath;
                        }
                    }
                }

            }
            else
            {
                printf("\nCannot get the registered task name: %x", hr);
            }
            //SysFreeString(bstrTaskPath);
            //SysFreeString(bstrTaskimg);
            pRegisteredTask->Release();
        }
        else
        {
            printf("\nCannot get the registered task item at index=%d: %x", i+1, hr);
        }
    }

    //子文件夹数量
    LONG num_sub_folders = 0;
    ITaskFolderCollection* pTaskSubFolders = NULL;

    /*
        目录下所有包含任务的子文件夹集合 GetFolders https://docs.microsoft.com/zh-cn/windows/win32/api/taskschd/nf-taskschd-itaskfolder-getfolders
        文件夹集合计数 get_Count https://docs.microsoft.com/zh-cn/windows/win32/api/taskschd/nn-taskschd-itaskfoldercollection
        */
    pSubFolder->GetFolders(0, &pTaskSubFolders);
    pTaskSubFolders->get_Count(&num_sub_folders);


    if(num_sub_folders == 0)
    {

        return *map_taskpath_imagepath;
    }
    else
    {
        qDebug()<<num_sub_folders;

        //遍历每个子文件夹
        for(int i = 0; i<num_sub_folders;i++)
        {
            //Gets the specified folder from the collection
            //https://docs.microsoft.com/zh-cn/windows/win32/api/taskschd/nf-taskschd-itaskfoldercollection-get_item
            ITaskFolder* pSubFolder = NULL;
            qDebug()<<num_sub_folders;

            //出现异常？指针问题？

            hr = pTaskSubFolders->get_Item( _variant_t(i+1), &pSubFolder );

            //成功遍历到子文件夹（不是这个原因异常）

            if( SUCCEEDED(hr) )
            {
                read_schedule_task_subfolder(map_taskpath_imagepath,pSubFolder);
            }

            pSubFolder->Release();

        }


    }


    // 2021年6月11日
    //释放根目录下任务集合
    pTaskCollection->Release();

    //取消COM初始化
    //CoUninitialize();
    return *map_taskpath_imagepath;
}



//读根目录C:\Windows\System32\Tasks
//引用传参
map<char*, char*> read_schedule_task_folder(map<char*, char*> *map_taskpath_imagepath)
{
    //  ------------------------------------------------------
    //  Initialize COM
    //CoInitializeEx都不行
    //HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);//单线程
    //HRESULT hr = CoInitializeEx(NULL,COINIT_MULTITHREADED);//多线程
    HRESULT hr = CoInitialize(NULL);
    if( FAILED(hr) )
    {
        printf("\nCoInitializeEx failed: %x", hr );
        CoUninitialize();
        return *map_taskpath_imagepath;
    }
    

    //HRESULT hr;

    //  Set general COM security levels.
    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL);

    if( FAILED(hr) )
    {
        printf("\nCoInitializeSecurity failed: %x", hr );
        //CoUninitialize();
        return *map_taskpath_imagepath;
    }

    //  ------------------------------------------------------
    //  Create an instance of the Task Service. 
    //可以连接到task scheduled程序服务，访问task文件夹
    ITaskService *pService = NULL;
    hr = CoCreateInstance( CLSID_TaskScheduler,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_ITaskService,
                           (void**)&pService );  
    if (FAILED(hr))
    {
          printf("Failed to CoCreate an instance of the TaskService class: %x", hr);
          //CoUninitialize();
          return *map_taskpath_imagepath;
    }
        
    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(),
        _variant_t(), _variant_t());
    if( FAILED(hr) )
    {
        printf("ITaskService::Connect failed: %x", hr );
        pService->Release();
        //CoUninitialize();
        return *map_taskpath_imagepath;
    }

    //  ------------------------------------------------------
    //  Get the pointer to the root task folder.

    //提供在文件夹里创建、删除任务的接口
    ITaskFolder *pRootFolder = NULL;
    hr = pService->GetFolder( _bstr_t( L"\\") , &pRootFolder );//Task根目录
    
    pService->Release();//connect完就可以释放
    if( FAILED(hr) )
    {
        printf("Cannot get Root Folder pointer: %x", hr );
        CoUninitialize();
        return *map_taskpath_imagepath;
    }

    //  -------------------------------------------------------
    //  Get the registered tasks in the folder.

    //已注册的任务集合 pTaskCollection
    IRegisteredTaskCollection* pTaskCollection = NULL;
    //ITaskFolderCollection* pTaskSubFolders = NULL;
    //LONG num_sub_folders = 0;
    //获取任务集合：TASK_ENUM_HIDDEN 可检索文件夹中包含隐藏任务的所有任务
    hr = pRootFolder->GetTasks( TASK_ENUM_HIDDEN, &pTaskCollection );
    //HRESULT hr1 = pRootFolder->GetFolders(0, &pTaskSubFolders);
    //pTaskSubFolders->get_Count(&num_sub_folders);
    //int a = num_sub_folders;
    //cout << a;
    //hr = pRootFolder->GetTasks( NULL, &pTaskCollection );

    //pRootFolder->Release(); //留待递归

    if( FAILED(hr) )
    {
        printf("Cannot get the registered tasks.: %x", hr);
        //CoUninitialize();
        return *map_taskpath_imagepath;
    }

    //任务数量
    LONG numTasks = 0;
    hr = pTaskCollection->get_Count(&numTasks);

    //若没有任务
    if( numTasks == 0 )
    {
        printf("\nNo Tasks are currently running" );
        pTaskCollection->Release();
        //CoUninitialize();
        return *map_taskpath_imagepath;
    }



    //遍历任务集合pTaskCollection
    for(LONG i=0; i < numTasks; i++)
    {
        //当前遍历到的任务 pRegisteredTask
        IRegisteredTask* pRegisteredTask = NULL;
        hr = pTaskCollection->get_Item( _variant_t(i+1), &pRegisteredTask );

        //成功遍历到任务 pRegisteredTask
        if( SUCCEEDED(hr) )
        {
            //任务路径
            BSTR bstrTaskPath = NULL;
            //任务的所有组件，例如任务设置、触发器、操作和注册信息
            ITaskDefinition* taskdef = NULL;
            //任务执行的操作
            IActionCollection* ppact = NULL;
            //所有操作对象继承的公共属性
            IAction* pact = NULL;
            //执行命令行操作
            IExecAction* peact = NULL;
            //
            BSTR bstrTaskimg = NULL;

            BSTR taskName = NULL;
            hr = pRegisteredTask->get_Name(&taskName);
            
            
            //成功取文件名            
            if( SUCCEEDED(hr) )
            {
                SysFreeString(taskName);
                //文件相对根目录的路径
                pRegisteredTask->get_Path(&bstrTaskPath);
                //任务定义
                pRegisteredTask->get_Definition(&taskdef);
                //从任务组件里获取任务执行的操作集合
                taskdef->get_Actions(&ppact);
                //从任务操作集合里获取指定操作到pact
                ppact->get_Item(1, &pact);
                //获取执行的imagepath
                pact->QueryInterface(IID_IExecAction, (void**)&peact);

				if (peact != NULL) 
                {
					//获取可执行文件的路径->char*
					hr = peact->get_Path(&bstrTaskimg);
					char* task_imagepath = ConvertBSTRToString(bstrTaskimg);
                    QString task_imagepath_qstr = charstr_to_QString(task_imagepath);
					//路径标准化
                    format_imagepath(&task_imagepath_qstr);

                    char* task_path = ConvertBSTRToString(bstrTaskPath);

					if (SUCCEEDED(hr)) 
                    {
                        // imagepath非空
						if (task_imagepath != "") 
                        {
                            //存入map一项
							(*map_taskpath_imagepath)[task_path] = task_imagepath;
						}
					}
				}

            }
            else
            {
                printf("\nCannot get the registered task name: %x", hr);
            }
            //SysFreeString(bstrTaskPath);
            //SysFreeString(bstrTaskimg);
            pRegisteredTask->Release();
        }
        else
        {
            printf("\nCannot get the registered task item at index=%d: %x", i+1, hr);
        }
    }
    
    //子文件夹数量
    LONG num_sub_folders = 0;
	ITaskFolderCollection* pTaskSubFolders = NULL;
	
    /*
        目录下所有包含任务的子文件夹集合 GetFolders https://docs.microsoft.com/zh-cn/windows/win32/api/taskschd/nf-taskschd-itaskfolder-getfolders
        文件夹集合计数 get_Count https://docs.microsoft.com/zh-cn/windows/win32/api/taskschd/nn-taskschd-itaskfoldercollection
        */
	pRootFolder->GetFolders(0, &pTaskSubFolders);
	pTaskSubFolders->get_Count(&num_sub_folders);
    
    
    if(num_sub_folders == 0)
    {

        return *map_taskpath_imagepath;
    }
    else
    {
        qDebug()<<num_sub_folders;

        //遍历每个子文件夹
        for(int i = 0; i<num_sub_folders;i++)
        {
            //Gets the specified folder from the collection
            //https://docs.microsoft.com/zh-cn/windows/win32/api/taskschd/nf-taskschd-itaskfoldercollection-get_item
            ITaskFolder* pSubFolder = NULL;
            qDebug()<<num_sub_folders;

            //出现异常？指针问题？

            hr = pTaskSubFolders->get_Item( _variant_t(i+1), &pSubFolder );

            //成功遍历到子文件夹（不是这个原因异常）

            if( SUCCEEDED(hr) )
            {
                read_schedule_task_subfolder(map_taskpath_imagepath,pSubFolder);
            }

            pSubFolder->Release();

        }


    }


    // 2021年6月11日 
    //释放根目录下任务集合
    pTaskCollection->Release();

    //取消COM初始化
    //CoUninitialize();
    return *map_taskpath_imagepath;
}








#endif // READ_SCHEDULE_TASKS_H
