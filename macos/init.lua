-- left+middle       = cut      (cmd+c, cmd+x)
-- left+right        = paste    (cmd+v)
-- right+left        = undo     (cmd+z)
-- right+middle      = redo     (cmd+shift+z)
-- middle+left       = return   (send return keypress)
-- middle+right      = space    (send space keypress)
--
-- middle+scrollDown = next app (cmd+tab)
-- middle+scrollUp   = prev app (cmd+shift+tab)

local eventtap   = require("hs.eventtap")
local eventTypes = eventtap.event.types
local mouse      = require("hs.mouse")
local logger     = hs.logger.new("mouse chording", "nothing")

-- declared in outer scope, but assigned in mouseStart()
local LMB
local MMB
local RMB
local keepLMB
local pureMMB
local stopMMB
local pureRMB
local stopRMB
local cycleApps

local leftDown = hs.eventtap.new({eventTypes.leftMouseDown}, function(e)
    logger:d("leftDown")
    -- right+left = undo
    if RMB then
        logger:d("undo")
        stopRMB = true
        return true, {
            eventtap.event.newKeyEvent({"cmd"}, "z", true),
            eventtap.event.newKeyEvent({"cmd"}, "z", false),
        } 
    end

    -- middle+left = return
    if MMB then
        logger:d("return")
        stopMMB = true
        return true, {
            eventtap.event.newKeyEvent({}, "return", true),
            eventtap.event.newKeyEvent({}, "return", false),
        } 
    end
     
    LMB = true
    return false
end)

local leftUp = hs.eventtap.new({eventTypes.leftMouseUp}, function(e)
    logger:d("leftUp")
    LMB = keepLMB
    keepLMB = false
    return false
end)

local middleDown = hs.eventtap.new({eventTypes.otherMouseDown}, function(e)
    logger:d("middleDown")
    if stopMMB then logger:d("stopMMB") end 
    -- don't interrupt pure middle click
    if pureMMB then
        logger:d("pureMMB")
        pureMMB = false
        return false
    end

    -- left+middle = copy and cut
    if LMB then
        logger:d("cut")
        keepLMB = true
        local mousePos = mouse.absolutePosition()
        return true, {
             eventtap.event.newMouseEvent(eventTypes.leftMouseUp, mousePos),
             eventtap.event.newKeyEvent({"cmd"}, "c", true),
             eventtap.event.newKeyEvent({"cmd"}, "c", false),
             eventtap.event.newKeyEvent({"cmd"}, "x", true),
             eventtap.event.newKeyEvent({"cmd"}, "x", false),
         } 
    end

    -- right+middle = redo
    if RMB then
        logger:d("redo")
        stopRMB = true
        return true, {
            eventtap.event.newKeyEvent({"cmd", "shift"}, "z", true),
            eventtap.event.newKeyEvent({"cmd", "shift"}, "z", false),
        } 
    end

    MMB = true
    return true
end)

local middleUp = hs.eventtap.new({eventTypes.otherMouseUp}, function(e)
    logger:d("middleUp")
    -- release cmd if window switching
    if cycleApps then
        logger:d("cycleApps")
        MMB = false
        stopMMB = false
        cycleApps = false
        return false, {
            eventtap.event.newKeyEvent(hs.keycodes.map.cmd, true),
            eventtap.event.newKeyEvent(hs.keycodes.map.cmd, false),
        }
    end

    -- nothing else clicked since otherMouseDown
    if not stopMMB and MMB then
        logger:d("pureMMB setup")
        MMB = false
        pureMMB = true
        local mousePos = mouse.absolutePosition()
        return false, {
            eventtap.event.newMouseEvent(eventTypes.otherMouseDown, mousePos),
            eventtap.event.newMouseEvent(eventTypes.otherMouseUp,   mousePos),
        }
    end

    stopMMB = false
    MMB = false
    return false
end)

local rightDown = hs.eventtap.new({eventTypes.rightMouseDown}, function(e)
    logger:d("rightDown")
    -- don't interrupt pure right click
    if pureRMB then
        pureRMB = false
        return false
    end

    -- left+right = paste
    if LMB then
        logger:d("paste")
       return true, {
            eventtap.event.newKeyEvent({"cmd"}, "v", true),
            eventtap.event.newKeyEvent({"cmd"}, "v", false),
        } 
    end

    -- middle+right = space
    if MMB then
        logger:d("space")
        stopMMB = true
        return true, {
            eventtap.event.newKeyEvent({}, "space", true),
            eventtap.event.newKeyEvent({}, "space", false),
        } 
    end

    RMB = true
    return true
end)

local rightUp = hs.eventtap.new({eventTypes.rightMouseUp}, function(e)
    logger:d("rightUp")
    -- nothing else clicked since rightMouseDown
    if not stopRMB and RMB then
        logger:d("pureRMB setup")
        RMB = false
        pureRMB = true
        local mousePos = mouse.absolutePosition()
        return false, {
            eventtap.event.newMouseEvent(eventTypes.rightMouseDown, mousePos),
            eventtap.event.newMouseEvent(eventTypes.rightMouseUp,   mousePos),
        }
    end

    stopRMB = false
    RMB = false
    return false
end)

local scrollWheel = hs.eventtap.new({eventTypes.scrollWheel}, function(e)
    logger:d("scrollWheel")
    -- MMB + scroll = cycle apps
    if MMB then
        cycleApps = true
        stopMMB = true
        local scroll = e:getProperty(hs.eventtap.event.properties["scrollWheelEventDeltaAxis1"])
        local mod = {"cmd"}
        if scroll > 0 then 
            logger:d("prev app")
            table.insert(mod, "shift")
        else
            logger:d("next app")
            scroll = scroll * -1
        end 
        if scroll > 3 then scroll = 3 end
        local switchEvents = {}
        for i = 1, scroll do
            table.insert(switchEvents, eventtap.event.newKeyEvent(mod, "tab", true))
            table.insert(switchEvents, eventtap.event.newKeyEvent(mod, "tab", false))
        end
        return true, switchEvents
    end

    -- RMB + scroll = cycle windows
    if RMB then
        local scroll = e:getProperty(hs.eventtap.event.properties["scrollWheelEventDeltaAxis1"])
        local mod = {"cmd"}
        if scroll > 0 then 
            logger:d("prev win")
            table.insert(mod, "shift")
        else
            logger:d("next win")
        end 
        stopRMB = true
        return true, {
            eventtap.event.newKeyEvent(mod, "'", true),
            eventtap.event.newKeyEvent(mod, "'", false),
        }
    end

    return false
end)

function mouseStart()
    logger:d("mouseStart")

    LMB        = false
    MMB        = false
    RMB        = false
    keepLMB    = false
    pureRMB    = false
    stopRMB    = false
    pureMMB    = false
    stopMMB    = false
    cycleApps  = false

    leftDown:start()
    leftUp:start()
    middleDown:start()
    middleUp:start()
    rightDown:start()
    rightUp:start()
    scrollWheel:start()
end

function mouseStop()
    logger:d("mouseStop")

    leftDown:stop()
    leftUp:stop()
    middleDown:stop()
    middleUp:stop()
    rightDown:stop()
    rightUp:stop()
    scrollWheel:stop()
end

mouseStart()

-- toogle with hyper + M
hs.hotkey.bind({"cmd", "alt", "ctrl", "shift"}, "M", function()
    logger:d("hyper + M")
    if leftDown:isEnabled() then
        hs.alert("mouse chording off")
        mouseStop()
    else
        hs.alert("mouse chording on")
        mouseStart()
    end
end)
