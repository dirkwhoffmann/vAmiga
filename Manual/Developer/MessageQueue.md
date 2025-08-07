# The Message Queue

In this document we will examine how vAmiga communicates with the graphical user interface. Sending commands from the GUI to the emulator core is simple. The GUI simply calls one of the API functions of the `Amiga` instance or one of its subcomponents.

The other way is a bit more sophisticated. The emulator communicates with the GUI via a message queue. The message queue is implemented by the `MsgQueue` class, which stores all pending messages in a ring buffer.

```c++
class MsgQueue : public SubComponent {

    // Ring buffer storing all pending messages
    util::RingBuffer <Message, 512> queue;

    ...
}
```

Class `MsgQueue` supports two different access methods: *Polling* or *Subscribing*.

## Polling the message queue

The message queue provides a public function `bool MsgQueue::get(Message &msg)` for reading a message. If the queue contains at least one message, `true` is returned and the oldest message is copied over to `msg`. If the queue is empty, the function returns `false`.

The polling method is utilized by [vAmiga.net](https://vamiganet.github.io), a web port of vAmiga based on WebAssembly. The corresponding JavaScript code looks like this: 

```js
export function doAnimationFrame()
{
    // Process pending messages
    while (1) {

        let msg = $amiga.readMessage();
        if (msg.type == 0) break;

        $proxy.processMsg(msg);
    }
}
```

Function `$amiga.readMessage()` is a small C++ wrapper around the `get` function:

```c++
Message AmigaProxy::readMessage()
{
    Message msg;

    if (!amiga->msgQueue.get(msg)) {
        msg.type = 0;
    }

    return msg;
}
```

On a positive note, polling ensures that message processing is always done in a separate thread. That is, it is never performed within the emulator thread itself. However, polling the queue is subject to potential message loss. If the GUI thread does not keep up with polling the queue, the ring buffer may overflow, resulting in lost messages. This drawback is solved by the subscription method, which we will discuss below.

## Subscribing to the message queue

As an alternative to storing pending messages in a ring buffer, the message queue also supports a callback mechanism. In this mode, sent messages are no longer written to the ring buffer. Instead, the registered callback function is called with the sent message as parameter.

Subscribing to the message queue works by specifying the callback function as an argument to the `Amiga::launch` function, which we have already discussed earlier:
```c++
void launch(const void *listener, Callback *func);
```
`Callback` is a type alias which is defined as follows:
```c++
typedef void Callback(const void *, Message);
```
Whenever the emulator sends a message, the provided callback function is invoked. The first argument is the raw pointer passed as the first argument to `Amiga::launch`. The second argument is an instance of the class `Message`. 

The subscription method is used by both the Mac version of vAmiga and *vAmiga Headless*. The latter is a small wrapper around the core emulator used mainly for testing the integrity of nightly builds. *vAmiga Headless* first creates an instance of the Amiga class on the stack: 
```c++
class Headless {

    ...

    // The emulator instance
    Amiga amiga;
}
```
Subscribing to the message queue happens in the `main` function:
```c++
int
Headless::main(int argc, char *argv[])
{
    ...

    // Launch the emulator thread
    amiga.launch(this, vamiga::process);
}
```
The first argument of the `launch` function is a pointer to the instance of the calling object. The second argument is a pointer to a global function called `process`, which is defined as follows:
```c++
void
process(const void *listener, Message msg)
{
    ((Headless *)listener)->process(msg);
}
```
The implementation of this function is fairly straightforward. It converts the provided raw pointer (which was set to `this` above) back to the correct type and calls the `process` function for that object. The implementation of this function looks like this:
```c++
void
Headless::process(Message msg)
{
    switch (msg.type) {
            
        case MSG_SCRIPT_DONE:
            ...

        }
    }
}
```
Subscribing to the message queue has the advantage of preventing any message from being lost. However, it is important to note that the message processing code is executed inside the emulator thread. This may cause problems if the message processing code calls functions of the public API of the `Amiga` class, since many of these functions assume that they are *not* called by the emulator thread itself. The Mac version of vAmiga solves this problem by passing all incoming messages to the GUI thread before execution. The relevant code is encapsulated in the `MyController.launch()` function, which is part of the Swift UI: 
```Swift
    func launch() {

        // Convert 'self' to a void pointer
        let myself = UnsafeRawPointer(Unmanaged.passUnretained(self).toOpaque())

        amiga.launch(myself) { (ptr, msg: Message) in

            // Convert void pointer back to 'self'
            let myself = Unmanaged<MyController>.fromOpaque(ptr!).takeUnretainedValue()

            // Process message in the main thread
            DispatchQueue.main.async {
                myself.processMessage(msg)
            }
        }
    }
```