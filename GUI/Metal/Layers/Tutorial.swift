
@MainActor
class TutorialLayerViewController: NSViewController {

    @IBOutlet weak var pageContainerView: NSView!
    var pageController: NSPageController!

    var objects: [String] = ["Step1", "Step2"]

    override func viewDidLoad() {
        super.viewDidLoad()

        pageController = NSPageController()
        pageController.delegate = self
        pageController.transitionStyle = .stackHistory
        pageController.arrangedObjects = objects

        addChild(pageController)
        pageContainerView.addSubview(pageController.view)
        pageController.view.frame = pageContainerView.bounds
        pageController.view.autoresizingMask = [.width, .height]
    }
}

extension TutorialLayerViewController: NSPageControllerDelegate {

    func pageController(_ pageController: NSPageController,
                        viewControllerForIdentifier identifier: String) -> NSViewController {

        let storyboard = NSStoryboard(name: "Tutorial", bundle: nil)
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

@MainActor
class Tutorial: Layer {

    var window: NSWindow { controller.window! }
    var contentView: NSView { window.contentView! }
    var storyboard: NSStoryboard { NSStoryboard(name: "Tutorial", bundle: nil) }

    var tutorialVC: TutorialLayerViewController!

    // var pageController: NSPageController!


    override init(renderer: Renderer) {

        super.init(renderer: renderer)

        tutorialVC = storyboard.instantiateController(withIdentifier: "TutorialLayerViewController") as? TutorialLayerViewController
        tutorialVC.view.wantsLayer = true
        tutorialVC.view.layer?.backgroundColor = NSColor.windowBackgroundColor.cgColor

        // Add as child view controller if needed
        /*
        renderer.parent.addChild(tutorialVC)
        renderer.parent.view.addSubview(tutorialVC.view)
        tutorialVC.view.frame = renderer.parent.view.bounds
        tutorialVC.view.autoresizingMask = [.width, .height]

        // Now configure the page controller inside it
        tutorialVC.pageController.delegate = tutorialVC  // or your layer
        tutorialVC.pageController.arrangedObjects = objects
         */

        /*
        pageController = storyboard.instantiateController(withIdentifier: "Tutorial") as? NSPageController

        pageController.delegate = self
        pageController.arrangedObjects = objects
        pageController.view.wantsLayer = true
        pageController.view.layer?.backgroundColor = NSColor.windowBackgroundColor.cgColor
        */
    }

    override func alphaDidChange() {

        tutorialVC.view.alphaValue = CGFloat(alpha.current)

        if alpha.current > 0 && tutorialVC.view.superview == nil {
            contentView.addSubview(tutorialVC.view)
            tutorialVC.view.frame = contentView.bounds
            tutorialVC.view.autoresizingMask = [.width, .height]
            // tutorialVC.navigateForward(nil) // show first page
        }

        if alpha.current == 0 && tutorialVC.view.superview != nil {
            tutorialVC.view.removeFromSuperview()
        }

        /*
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
         */
    }
}
