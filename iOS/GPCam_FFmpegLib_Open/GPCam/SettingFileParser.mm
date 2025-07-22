//
//  SettingFileParser.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/13/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "SettingFileParser.h"
#import "tinyxml2.h"
#import <stdio.h>
#import <stdlib.h>

using namespace tinyxml2;

//------------------------------------------------------------------------------------------------
@interface SettingFileParser ()
{

}


@end

//------------------------------------------------------------------------------------------------
@implementation SettingFileParser
//------------------------------------------------------------------------------------------------
-(id)init
{
    if ((self = [super init])) {
        self.Categories = [[NSMutableArray alloc]init];
    }
    return self;
    
}

//------------------------------------------------------------------------------------------------
-(bool) ParseValues:(SettingsItem *)SettingI
     withValuesNode:(XMLElement*)Values
{
    
    if(Values!=NULL)
    {
        XMLElement* Value = Values->FirstChildElement();
        while(Value!=NULL)
        {
            ValuesItem *NewValueI = [[ValuesItem alloc]init];
            NewValueI.Name = [NSString stringWithCString:Value->FirstChildElement("Name")->GetText() encoding:NSUTF8StringEncoding];
            NewValueI.ID   = (unsigned int)strtol(Value->FirstChildElement("ID")->GetText(),NULL,16);

            [SettingI.Values addObject:NewValueI];
            Value = Value->NextSiblingElement();
        }
        
    }
    
    return true;
}

//------------------------------------------------------------------------------------------------
-(bool) ParseSettings:(CategoriesItem *)CategoryI
     withSettingsNode:(XMLElement*)Settings
{
    
    if(Settings!=NULL)
    {
        XMLElement* Setting = Settings->FirstChildElement();
        while(Setting!=NULL)
        {
            SettingsItem *NewSettingI = [[SettingsItem alloc]init];
            NewSettingI.Name = [NSString stringWithCString:Setting->FirstChildElement("Name")->GetText() encoding:NSUTF8StringEncoding];
            NewSettingI.ID   = (unsigned int)strtol(Setting->FirstChildElement("ID")->GetText(),NULL,16);
            NewSettingI.Type = (unsigned int)strtol(Setting->FirstChildElement("Type")->GetText(),NULL,16);
            
            if(Setting->FirstChildElement("Default"))
            {
                NewSettingI.ValueString = [NSString stringWithCString:Setting->FirstChildElement("Default")->GetText() encoding:NSUTF8StringEncoding];
                NewSettingI.Current = (unsigned int)strtol(Setting->FirstChildElement("Default")->GetText(),NULL,16);
            }
            
            if(Setting->FirstChildElement("Reflash"))
                NewSettingI.ReFlash = (unsigned int)strtol(Setting->FirstChildElement("Reflash")->GetText(),NULL,16);
            
            XMLElement* Values = Setting->FirstChildElement("Values");
            if(![self ParseValues:NewSettingI withValuesNode:Values])
                return false;
            
            [CategoryI.Settings addObject:NewSettingI];
            Setting = Setting->NextSiblingElement();
        }
        
    }
    
    return true;
}
//------------------------------------------------------------------------------------------------
-(bool) ParseCategories:(XMLElement*)Categories
{
    
    if(Categories!=NULL)
    {
        XMLElement* Category = Categories->FirstChildElement();
        while(Category!=NULL)
        {
            CategoriesItem *NewCategoryI = [[CategoriesItem alloc]init];
            NewCategoryI.Name = [NSString stringWithCString:Category->FirstChildElement("Name")->GetText() encoding:NSUTF8StringEncoding];
            XMLElement* Settings = Category->FirstChildElement("Settings");
            if(![self ParseSettings:NewCategoryI withSettingsNode:Settings])
                return false;
            
            [_Categories addObject:NewCategoryI];
            Category = Category->NextSiblingElement();
        }
        
    }
    
    return true;
}
//------------------------------------------------------------------------------------------------
-(bool) Parsefile:(NSString *)filePath
{

    XMLDocument doc;
    if(doc.LoadFile([filePath UTF8String])!=XML_SUCCESS)
    {
        printf("Error: Failed to read file \"%s\"!!\n",[filePath UTF8String]);
        return false;
    }
    
    [_Categories removeAllObjects];

    XMLElement* pMenu = doc.RootElement();
    if ( pMenu )
    {
        _Version = [NSString stringWithCString:pMenu->Attribute("version") encoding:NSUTF8StringEncoding];
        XMLElement* Categories = pMenu->FirstChildElement();
        if(![self ParseCategories:Categories])
            return false;
    }
    
    [self GetDeviceFWVersion];
    
    return true;
}
//------------------------------------------------------------------------------------------------
-(NSString*) GetValueStrFromSettingFile:(unsigned int)i32SettingID
                             ValueIndex:(int)i32Value
{
    
    for(CategoriesItem *Category in self.Categories)
    {
        for( SettingsItem *Setting in Category.Settings)
        {
            if(Setting.ID == i32SettingID)
            {
                if (Setting.Values == nil || Setting.Values.count <= i32Value) {
                    return @"";
                }
                ValuesItem *Value = [Setting.Values  objectAtIndex:i32Value];
                if(Value)
                    return Value.Name;
                else
                    return @"";
            }
        }
    }
    
     return @"";
}
//------------------------------------------------------------------------------------------------
-(int)  GetValueFromSettingFile:(unsigned int)i32SettingID
{
    
    for (CategoriesItem *CategoriesI in self.Categories)
    {
        for (SettingsItem *SettingsI in CategoriesI.Settings)
        {
            if(SettingsI.ID == i32SettingID)
            {
                return SettingsI.Current;
            }
        }
    }
    
    return -1;
}
//------------------------------------------------------------------------------------------------
-(void) GetDeviceFWVersion
{

    NSString *pString = [self GetValueStrFromSettingFile:Version_Setting_ID
                                              ValueIndex:Version_Value_Index];
    
    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:@"(\\d+) V(.*)" options:NSRegularExpressionCaseInsensitive error:nil];
    
    [regex enumerateMatchesInString:pString options:0
                              range:NSMakeRange(0, [pString length])
                         usingBlock:^(NSTextCheckingResult *result, NSMatchingFlags flags, BOOL *stop)
     {
         
         if([result numberOfRanges]>=3)
         {
             NSString *pDate    = [pString substringWithRange:[result rangeAtIndex:1]];
             NSString *pVersion = [pString substringWithRange:[result rangeAtIndex:2]];
             
             NSArray *pVersionSplit = [pVersion componentsSeparatedByString:@"."];
             
             _FWVersion = 0;
             int i32Shift = 24;
             for(NSString *currentNumberString in pVersionSplit)
             {
                 _FWVersion |= ([currentNumberString intValue] << i32Shift);
                 i32Shift-=8;
                 if(i32Shift<0)
                     break;
             }
             
             
             printf("Device FW version: %X\n",_FWVersion);
             
         }
         
     }];

}

@end
