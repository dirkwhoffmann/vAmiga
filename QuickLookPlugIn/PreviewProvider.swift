//
//  PreviewProvider.swift
//  QuickLookPlugIn
//
//  Created by Dirk Hoffmann on 04.02.25.
//  Copyright © 2025 Dirk Hoffmann. All rights reserved.
//

/*
import Cocoa
import Quartz

class PreviewProvider: QLPreviewProvider, QLPreviewingController {
    

    /*
     Use a QLPreviewProvider to provide data-based previews.
     
     To set up your extension as a data-based preview extension:

     - Modify the extension's Info.plist by setting
       <key>QLIsDataBasedPreview</key>
       <true/>
     
     - Add the supported content types to QLSupportedContentTypes array in the extension's Info.plist.

     - Change the NSExtensionPrincipalClass to this class.
       e.g.
       <key>NSExtensionPrincipalClass</key>
       <string>$(PRODUCT_MODULE_NAME).PreviewProvider</string>
     
     - Implement providePreview(for:)
     */
    
    func providePreview(for request: QLFilePreviewRequest) async throws -> QLPreviewReply {
    
        //You can create a QLPreviewReply in several ways, depending on the format of the data you want to return.
        //To return Data of a supported content type:
        
        NSLog("dirkwhoffmann: loadView() ")
        
        let contentType = UTType.plainText // replace with your data type
        // let contentType = UTType("de.dirkwhoffmann.retro.vamiga")!

        let reply = QLPreviewReply.init(dataOfContentType: contentType, contentSize: CGSize.init(width: 200, height: 200)) { (replyToUpdate : QLPreviewReply) in

            let data = Data("Holla, die Waldfee".utf8)
            
            //setting the stringEncoding for text and html data is optional and defaults to String.Encoding.utf8
            replyToUpdate.stringEncoding = .utf8
            
            //initialize your data here
            
            return data
        }
              
        // <string>$(PRODUCT_MODULE_NAME).PreviewViewController</string>
        
        return reply
    }
}
*/
