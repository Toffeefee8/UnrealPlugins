These plugins are inteded to ease the development in Unreal Engine 5. All these plugins also serve as code examples.

**Object Extensions**

Object Extensions is an Unreal Engine plugin that augments the core Unreal libraries with a suite of utility objects. It features built-in casted getters for core components and timelines that work even within actor components. Additionally, it provides utility structs—such as distance and time structs—that simplify runtime unit conversion and enhance the editor's user experience. With object serialization utilities and support for replicated UObjects, every aspect of this plugin is designed to streamline development and improve productivity in Unreal Engine.

**Debug System**

The Debug System is a plugin that links debug messages to gameplay tags, allowing for precise control over debugging output. Developers can manage these messages globally from the editor preferences—choosing to display, suppress, or format them as needed. This targeted approach not only refines the debugging process but also enhances team collaboration by enabling individuals to hide debug outputs that are irrelevant to their current tasks.

**Region System**

The Region System is a robust framework for tracking player locations and partitioning the game map into designated regions. Configurable within the editor, it can be baked directly into world objects to eliminate the need for runtime registration. The system broadcasts global events for seamless object tracking and classification, making it easy to integrate with other gameplay systems. Additionally, region modules—such as an electricity module—extend its capabilities by creating hierarchical networks that distribute power and control objects based on their current power states.

**Save System**

The Save System is an enhanced extension of Unreal’s native save objects. Operating as a runtime subsystem, it manages both world and player saves, allowing objects to autonomously request saving or loading of their state. An integrated editor utility offers developers a powerful interface to inspect and manipulate saved data, thereby simplifying debugging and data management during development.
