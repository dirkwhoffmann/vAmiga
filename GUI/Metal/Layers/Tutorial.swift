@MainActor
class Tutorial: Layer {

    var window: NSWindow { controller.window! }
    var contentView: NSView { window.contentView! }
    var storyboard: NSStoryboard { NSStoryboard(name: "Tutorial", bundle: nil) }

    var pageController: NSPageController!
    var objects: [String] = ["Step1", "Step2"]

    override init(renderer: Renderer) {

        super.init(renderer: renderer)

        pageController = storyboard.instantiateController(withIdentifier: "Tutorial") as? NSPageController

        pageController.delegate = self
        pageController.arrangedObjects = objects
        pageController.view.wantsLayer = true
        pageController.view.layer?.backgroundColor = NSColor.windowBackgroundColor.cgColor
    }

    override func alphaDidChange() {

        pageController.view.alphaValue = CGFloat(alpha.current)

        if alpha.current > 0 && pageController.view.superview == nil {
            contentView.addSubview(pageController.view)
            pageController.view.frame = contentView.bounds
            pageController.view.autoresizingMask = [.width, .height]
            pageController.navigateForward(nil) // show first page
        }

        if alpha.current == 0 && pageController.view.superview != nil {
            pageController.view.removeFromSuperview()
        }
    }

}

extension Tutorial: NSPageControllerDelegate {

    func pageController(_ pageController: NSPageController,
                        viewControllerForIdentifier identifier: String) -> NSViewController {

        return storyboard.instantiateController(withIdentifier: identifier) as! NSViewController
    }

    func pageController(_ pageController: NSPageController, identifierFor object: Any) -> String {

        return object as! String
    }

    func pageController(_ pageController: NSPageController, didTransitionTo object: Any) {

        if let id = object as? String, id == objects.last {

            print("Finish...")
        }
    }
}
