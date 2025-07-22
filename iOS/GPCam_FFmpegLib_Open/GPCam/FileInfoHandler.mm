//
//  FileInfoHandler.m
//  GPCam
//
//  Created by generalplus_sa1 on 9/14/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "FileInfoHandler.h"
#import "tinyxml2.h"
#import <stdio.h>
#import <stdlib.h>

#define FILEINO_NAME         "FileInfo.xml"

using namespace tinyxml2;

//------------------------------------------------------------------------------------------------
@interface FileInfoHandler ()
{
    
}

@property (nonatomic, strong) NSMutableArray *InfoArray;
@end

//------------------------------------------------------------------------------------------------
@implementation FileInfoHandler

//------------------------------------------------------------------------------------------------
-(id)init
{
    if ((self = [super init])) {
        self.InfoArray = [[NSMutableArray alloc]init];
    }
    return self;
    
}
//------------------------------------------------------------------------------------------------
-(bool) ReadFile
{
    
    NSString *docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSString *filePath = [docsPath stringByAppendingPathComponent:[NSString stringWithCString:FILEINO_NAME encoding:NSUTF8StringEncoding]];
    
    XMLDocument doc;
    if(doc.LoadFile([filePath UTF8String])!=XML_SUCCESS)
    {
        printf("Error: Failed to read file \"%s\"!!\n",[filePath UTF8String]);
        return false;
    }
    
    [_InfoArray removeAllObjects];
    
    XMLElement* pRoot = doc.RootElement(); //FileInfoList
    if ( pRoot )
    {
        XMLElement* FileInfo = pRoot->FirstChildElement();//FileInfo
        while(FileInfo!=NULL)
        {
            FileInfoItem *NewInfo = [[FileInfoItem alloc]init];
            NewInfo.FileName = [NSString stringWithCString:FileInfo->FirstChildElement("FileName")->GetText() encoding:NSUTF8StringEncoding];
            
            XMLElement *assertURLE = FileInfo->FirstChildElement("assertURL");
            if(assertURLE)
            {
                if(assertURLE->GetText())
                {
                    NSString *assertURLS = [NSString stringWithCString:assertURLE->GetText() encoding:NSUTF8StringEncoding];
                    NewInfo.assertURL = [NSURL URLWithString:[assertURLS stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
                }
            }
            
            NewInfo.lDate   = strtol(FileInfo->FirstChildElement("Date")->GetText(),NULL,16);
            NewInfo.lTime   = strtol(FileInfo->FirstChildElement("Time")->GetText(),NULL,16);
            NewInfo.lSize   = strtol(FileInfo->FirstChildElement("Size")->GetText(),NULL,16);
            [_InfoArray addObject:NewInfo];
            FileInfo = FileInfo->NextSiblingElement();
        }
    }
    
    
    return true;
}
//------------------------------------------------------------------------------------------------
-(bool) SaveFile
{
    
    NSString *docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSString *filePath = [docsPath stringByAppendingPathComponent:[NSString stringWithCString:FILEINO_NAME encoding:NSUTF8StringEncoding]];
    
    XMLDocument doc;
    
    XMLDeclaration * decl = doc.NewDeclaration();
    doc.LinkEndChild(decl);

    XMLElement * FileList = doc.NewElement("FileInfoList");
    doc.LinkEndChild( FileList );
    char szTest[64];
    
    for (FileInfoItem* Existed in _InfoArray)
    {
        XMLElement * FileInfo = doc.NewElement("FileInfo");
    
        XMLElement * FileName = doc.NewElement("FileName");
        XMLText *FileNameText = doc.NewText([Existed.FileName UTF8String]);
        FileName->LinkEndChild(FileNameText);
        
        XMLElement * assertURL = doc.NewElement("assertURL");
        if(Existed.assertURL)
        {
            XMLText *assertURLText = doc.NewText([[Existed.assertURL absoluteString]UTF8String]);
            assertURL->LinkEndChild(assertURLText);
        }
        
        XMLElement * Data = doc.NewElement("Date");
        sprintf(szTest,"%lX",Existed.lDate);
        XMLText *DataText = doc.NewText(szTest);
        Data->LinkEndChild(DataText);
     
        XMLElement *Time = doc.NewElement("Time");
        sprintf(szTest,"%lX",Existed.lTime);
        XMLText *TimeText = doc.NewText(szTest);
        Time->LinkEndChild(TimeText);
        
        XMLElement * Size = doc.NewElement("Size");
        sprintf(szTest,"%lX",Existed.lSize);
        XMLText *SizeText = doc.NewText(szTest);
        Size->LinkEndChild(SizeText);
        
        FileInfo->LinkEndChild(FileName);
        FileInfo->LinkEndChild(assertURL);
        FileInfo->LinkEndChild(Data);
        FileInfo->LinkEndChild(Time);
        FileInfo->LinkEndChild(Size);
        FileList->LinkEndChild(FileInfo);
    }
    
    
    if(doc.SaveFile([filePath UTF8String])!=XML_SUCCESS)
    {
        printf("Error: Failed to save file \"%s\"!!\n",[filePath UTF8String]);
        return false;
    }
    
    
    return true; 
}
//------------------------------------------------------------------------------------------------
-(bool) AddItem:(FileInfoItem*)item
{
    if([self CheckAndReplaceItem:item]!=CHECK_RET_NOFOUND)
        return true;
    
    [_InfoArray addObject:item];
    return true;
}
//------------------------------------------------------------------------------------------------
-(int) CheckAndReplaceItem:(FileInfoItem*)item
{
    
    for (FileInfoItem* Existed in _InfoArray)
    {
        if([item.FileName isEqualToString:Existed.FileName])
        {
            if(item.lDate == Existed.lDate &&
               item.lSize == Existed.lSize &&
               item.assertURL == Existed.assertURL )
            {
                return CHECK_RET_MATCH;
            }
            else
            {
                //replace
                Existed.lDate = item.lDate;
                Existed.lSize = item.lSize;
                Existed.assertURL = item.assertURL;
                return CHECK_RET_MISMATCH;
            }
        }
        
    }
    
    return CHECK_RET_NOFOUND;
}
//------------------------------------------------------------------------------------------------
-(NSURL*) GetFileAssertUrl:(NSString*) FileName
{
    for (FileInfoItem* Existed in _InfoArray)
        if([FileName isEqualToString:Existed.FileName])
            return Existed.assertURL;
    return nil;
}
//------------------------------------------------------------------------------------------------

@end
