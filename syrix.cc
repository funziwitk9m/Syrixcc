--======================= GLOBAL SETTINGS =======================
getgenv().tbEnabled      = true
getgenv().tbDelay        = 0.05
getgenv().tbKeybind      = Enum.KeyCode.Q
getgenv().tbMode         = "Toggle"
getgenv().tbPrediction   = 0
getgenv().hitboxRadius   = 1
getgenv().knifeCheck     = true

getgenv().tbNotify       = true
getgenv().hbNotify       = true
getgenv().wsNotify       = true

getgenv().syrixHitBoxEpanderSettings = {
    Enabled     = false,
    Size        = 2,
    RefreshTime = 2
}

getgenv().walkSpeedSettings = {
    WalkSpeed = {Enabled = true, Speed = 400},
    Activation = {WalkSpeedToggleKey = "T"}
}

local uis           = game:GetService("UserInputService")
local Players       = game:GetService("Players")
local LocalPlayer   = Players.LocalPlayer
local RunService    = game:GetService("RunService")
local Camera        = workspace.CurrentCamera
local defaultSpeed  = 16

local ScriptRunning = true
local typingInChat  = false
local killed        = false
local originalPartData = {}

--======================= UTILITIES =======================
local function notify(msg, duration, which)
    which = which or "tb"
    if (which=="tb" and getgenv().tbNotify) or 
       (which=="hb" and getgenv().hbNotify) or 
       (which=="ws" and getgenv().wsNotify) then
        pcall(function()
            game.StarterGui:SetCore("SendNotification", {
                Title = "Syrix",
                Text = msg,
                Duration = duration or 3
            })
        end)
    end
end

local function predictPos(part, multiplier)
    if not part then return nil end
    return part.Position + (part.Velocity * multiplier)
end

--======================= WALK SPEED =======================
-- Walkspeed logic
local isSpeedEnabled = false
local speedConnection

local function updateSpeed()
    if LocalPlayer.Character and LocalPlayer.Character:FindFirstChild("Humanoid") then
        if isSpeedEnabled and getgenv().walkSpeedSettings.WalkSpeed.Enabled then
            LocalPlayer.Character.Humanoid.WalkSpeed = getgenv().walkSpeedSettings.WalkSpeed.Speed
        else
            LocalPlayer.Character.Humanoid.WalkSpeed = defaultSpeed
        end
    end
end

-- Walkspeed connection
speedConnection = RunService.RenderStepped:Connect(updateSpeed)
LocalPlayer.CharacterAdded:Connect(function(character)
    character:WaitForChild("Humanoid")
    updateSpeed()
end)
uis.InputBegan:Connect(function(input, processed)
    if processed then return end
    if not killed and input.KeyCode == Enum.KeyCode[getgenv().walkSpeedSettings.Activation.WalkSpeedToggleKey] then
        isSpeedEnabled = not isSpeedEnabled
        updateSpeed()
        notify(isSpeedEnabled and "WalkSpeed ON" or "WalkSpeed OFF", 2.5, "ws")
    end
end)

--======================= TRIGGERBOT =======================
-- ⚙️ Services
local Players = game:GetService("Players")
local RunService = game:GetService("RunService")
local UserInput = game:GetService("UserInputService")
local Camera = workspace.CurrentCamera
local LocalPlayer = Players.LocalPlayer

-- ⚙️ Getgenv Config
getgenv().tbEnabled = getgenv().tbEnabled or true
getgenv().hitboxRadius = getgenv().hitboxRadius or 6
getgenv().tbPrediction = getgenv().tbPrediction or 0.08

-- 🔹 Cached Player List for Performance
local allPlayers = {}

local function updatePlayerList()
    allPlayers = {}
    for _, plr in ipairs(Players:GetPlayers()) do
        if plr ~= LocalPlayer then
            table.insert(allPlayers, plr)
        end
    end
end

updatePlayerList()
Players.PlayerAdded:Connect(updatePlayerList)
Players.PlayerRemoving:Connect(updatePlayerList)

-- 🔹 Lightweight Predictive Function
local function predictPos(part, delta)
    local root = part
    if root and root:IsA("BasePart") then
        return root.Position + (root.Velocity * delta)
    end
    return part.Position
end

-- 🔹 Optimized Closest Body Part Finder
local function getClosestBodyPart()
    local mouse = UserInput:GetMouseLocation()
    local ray = Camera:ViewportPointToRay(mouse.X, mouse.Y)
    local origin, direction = ray.Origin, ray.Direction
    local radius = getgenv().hitboxRadius
    local bestPart, bestDist = nil, radius

    for _, player in ipairs(allPlayers) do
        local char = player.Character
        if char then
            local head = char:FindFirstChild("Head")
            local root = char:FindFirstChild("HumanoidRootPart")
            if head or root then
                for _, part in ipairs({head, root}) do
                    if part then
                        local predicted = predictPos(part, getgenv().tbPrediction)
                        local directionOffset = predicted - origin
                        local projection = direction:Dot(directionOffset)
                        if projection > 0 then
                            local closestPoint = origin + direction * projection
                            local dist = (predicted - closestPoint).Magnitude
                            if dist < bestDist then
                                bestDist = dist
                                bestPart = part
                            end
                        end
                    end
                end
            end
        end
    end

    return bestPart
end

-- 🔹 Knife Check Function
local function hasKnifeEquipped()
    local char = LocalPlayer.Character
    if not char then return false end
    local tool = char:FindFirstChildOfClass("Tool")
    return tool and tool.Name:lower():find("knife", 1, true) ~= nil
end

-- 🔹 Reaction Loop (Runs Every 50ms to Save FPS)
task.spawn(function()
    while task.wait(0.05) do
        if getgenv().tbEnabled and (not getgenv().knifeCheck or hasKnifeEquipped()) then
            local target = getClosestBodyPart()
            if target then
                -- Replace this with your tool/fire logic
                mouse1click() -- e.g. automated click
            end
        end
    end
end)




-- triggerbot connection
uis.InputBegan:Connect(function(input, processed)
    if processed or killed then return end
    if input.KeyCode == getgenv().tbKeybind then
        if getgenv().tbMode == "Toggle" then
            tbActive = not tbActive
            getgenv().tbEnabled = tbActive
            notify(tbActive and "Triggerbot ON" or "Triggerbot OFF", 2.5, "tb")
        elseif getgenv().tbMode == "Hold" then
            tbActive = true
            getgenv().tbEnabled = tbActive
            notify("Triggerbot ON", 2.5, "tb")
        end
    end
    if uis:IsKeyDown(Enum.KeyCode.RightControl) and input.KeyCode == Enum.KeyCode.RightShift then
        killed = true
        getgenv().syrixHitBoxEpanderSettings.Enabled = false
        notify("Syrix: Killed", 3)
        if speedConnection then speedConnection:Disconnect() end
        isSpeedEnabled = false
        if LocalPlayer.Character and LocalPlayer.Character:FindFirstChild("Humanoid") then
            LocalPlayer.Character.Humanoid.WalkSpeed = defaultSpeed
        end
        for part, data in pairs(originalPartData) do
            if part and part.Parent then
                part.Size = data.Size
                part.Transparency = data.Transparency
            end
        end
        originalPartData = {}
        local pgui = LocalPlayer:FindFirstChildOfClass("PlayerGui")
        if pgui and pgui:FindFirstChild("SyrixUI") then
            pgui.SyrixUI:Destroy()
        end
    end
    if input.KeyCode == Enum.KeyCode.F6 then
        killed = true
        getgenv().syrixHitBoxEpanderSettings.Enabled = false
        notify("Syrix: Killed", 3)
        if speedConnection then speedConnection:Disconnect() end
        isSpeedEnabled = false
        if LocalPlayer.Character and LocalPlayer.Character:FindFirstChild("Humanoid") then
            LocalPlayer.Character.Humanoid.WalkSpeed = defaultSpeed
        end
        for part, data in pairs(originalPartData) do
            if part and part.Parent then
                part.Size = data.Size
                part.Transparency = data.Transparency
            end
        end
        originalPartData = {}
        local pgui = LocalPlayer:FindFirstChildOfClass("PlayerGui")
        if pgui and pgui:FindFirstChild("SyrixUI") then
            pgui.SyrixUI:Destroy()
        end
    end
end)
uis.InputEnded:Connect(function(input)
    if input.KeyCode == getgenv().tbKeybind and getgenv().tbMode == "Hold" then
        tbActive = false
        getgenv().tbEnabled = false
        notify("Triggerbot OFF", 2.5, "tb")
    end
end)
RunService.RenderStepped:Connect(function()
    if not ScriptRunning or not tbActive or killed then return end
    if typingInChat then return end
    if getgenv().knifeCheck and hasKnifeEquipped() then return end
    local targetPart = getClosestBodyPart()
    if targetPart then
        pcall(mouse1press)
        task.wait(0.03)
        pcall(mouse1release)
        if getgenv().tbDelay > 0 then
            task.wait(getgenv().tbDelay)
        end
    end
end)

--======================= HITBOX EXPANDER =======================
-- hitbox logic/connection
task.spawn(function()
    while getgenv().syrixHitBoxEpanderSettings.Enabled and not killed do
        for _, Player in ipairs(Players:GetPlayers()) do
            if Player ~= LocalPlayer and Player.Character then
                for _, part in ipairs(Player.Character:GetChildren()) do
                    if part:IsA("BasePart") then
                        if not originalPartData[part] then
                            originalPartData[part] = {
                                Size = part.Size,
                                Transparency = part.Transparency
                            }
                        end
                        part.Size = Vector3.new(
                            getgenv().syrixHitBoxEpanderSettings.Size,
                            getgenv().syrixHitBoxEpanderSettings.Size,
                            getgenv().syrixHitBoxEpanderSettings.Size
                        )
                        part.Transparency = 1
                    end
                end
            end
        end
        task.wait(getgenv().syrixHitBoxEpanderSettings.RefreshTime)
    end
end)


-- ==================== MODERN TAB GUI ====================
local guiParent = LocalPlayer:FindFirstChildOfClass("PlayerGui") or LocalPlayer:WaitForChild("PlayerGui")
local existing = guiParent:FindFirstChild("SyrixUI")
if existing then existing:Destroy() end

local COLORS = { bgDark=Color3.fromRGB(10,10,12), panelDark=Color3.fromRGB(16,16,20), text=Color3.fromRGB(235,235,240), strokeSoft=Color3.fromRGB(52,52,60), blue=Color3.fromRGB(24,153,255) }
local screen=Instance.new("ScreenGui",guiParent) screen.Name="SyrixUI" screen.ResetOnSpawn=false
local main=Instance.new("Frame",screen)
main.BackgroundColor3=COLORS.panelDark main.Size=UDim2.fromOffset(580,380) main.Position=UDim2.fromScale(0.5,0.5) main.AnchorPoint=Vector2.new(0.5,0.5)
local corner=Instance.new("UICorner",main) corner.CornerRadius=UDim.new(0,16)
local stroke=Instance.new("UIStroke",main) stroke.Color=COLORS.strokeSoft stroke.Thickness=2 stroke.Transparency=0.13

-- Tabs
local tabFrame=Instance.new("Frame",main) tabFrame.Size=UDim2.new(1,0,0,38) tabFrame.Position=UDim2.fromOffset(0,0) tabFrame.BackgroundTransparency=1
local tabSelected=Instance.new("NumberValue",tabFrame) tabSelected.Value=1

local function makeTabParent()
    local c=Instance.new("Frame",main)
    c.Size=UDim2.new(1,-32,1,-54) c.Position=UDim2.fromOffset(16,44) c.BackgroundColor3=COLORS.bgDark
    Instance.new("UICorner",c).CornerRadius=UDim.new(0,13)
    Instance.new("UIStroke",c).Color=COLORS.strokeSoft
    c.Visible=false
    return c
end

local tabNames = {"Triggerbot","HB Expander","Walkspeed"}
local tabCons = {}
local tabBtns = {}
for i,n in ipairs(tabNames) do
    local btn = Instance.new("TextButton",tabFrame)
    btn.Text = n
    btn.Font = Enum.Font.GothamBold
    btn.TextColor3 = i==1 and COLORS.blue or COLORS.text
    btn.TextSize = 16
    btn.BackgroundColor3 = i==1 and COLORS.bgDark or COLORS.panelDark
    btn.Size = UDim2.new(0,116,1,0)
    btn.Position = UDim2.new(0,(i-1)*120,0,0)
    Instance.new("UICorner",btn).CornerRadius = UDim.new(0,9)
    btn.MouseButton1Click:Connect(function()
        tabSelected.Value = i
        for j = 1,#tabNames do
            tabBtns[j].TextColor3 = COLORS.text
            tabCons[j].Visible = false
            tabBtns[j].BackgroundColor3 = COLORS.panelDark
        end
        btn.TextColor3 = COLORS.blue
        btn.BackgroundColor3 = COLORS.bgDark
        tabCons[i].Visible = true
    end)
    tabBtns[i]=btn
    local c=makeTabParent()
    c.Visible = (i==1)
    tabCons[i]=c
end

-- Triggerbot Tab
local tbc = tabCons[1]
local trigSwitch = Instance.new("TextButton",tbc)
trigSwitch.Size=UDim2.new(0,90,0,26) trigSwitch.Position=UDim2.fromOffset(20,24)
trigSwitch.Text = getgenv().tbEnabled and "Enabled" or "Disabled"
trigSwitch.Font=Enum.Font.GothamBold trigSwitch.TextSize=14
trigSwitch.BackgroundColor3 = getgenv().tbEnabled and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
Instance.new("UICorner",trigSwitch).CornerRadius=UDim.new(0,8)
trigSwitch.MouseButton1Click:Connect(function()
    getgenv().tbEnabled = not getgenv().tbEnabled
    trigSwitch.Text = getgenv().tbEnabled and "Enabled" or "Disabled"
    trigSwitch.BackgroundColor3 = getgenv().tbEnabled and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
end)

local tbDelayLbl=Instance.new("TextLabel",tbc) tbDelayLbl.Position=UDim2.new(0,20,0,65) tbDelayLbl.Size=UDim2.new(0,80,0,19)
tbDelayLbl.Text="Delay:" tbDelayLbl.BackgroundTransparency=1 tbDelayLbl.TextColor3=COLORS.text tbDelayLbl.Font=Enum.Font.Gotham tbDelayLbl.TextSize=14

local tbDelayBox = Instance.new("TextBox", tbc) tbDelayBox.Size=UDim2.new(0,60,0,26)
tbDelayBox.Position=UDim2.new(0,100,0,65) tbDelayBox.Text = tostring(getgenv().tbDelay) tbDelayBox.Font=Enum.Font.Gotham tbDelayBox.TextSize=14
Instance.new("UICorner",tbDelayBox).CornerRadius=UDim.new(0,8)
tbDelayBox.FocusLost:Connect(function()
    local val=tonumber(tbDelayBox.Text) if val then getgenv().tbDelay=val else tbDelayBox.Text=tostring(getgenv().tbDelay) end
end)

-- Triggerbot Keybind changer
local tbKeybindLbl=Instance.new("TextLabel",tbc) tbKeybindLbl.Position=UDim2.new(0,200,0,22) tbKeybindLbl.Size=UDim2.new(0,90,0,22)
tbKeybindLbl.Text="Triggerbot Key" tbKeybindLbl.BackgroundTransparency=1 tbKeybindLbl.TextColor3=COLORS.text tbKeybindLbl.Font=Enum.Font.Gotham tbKeybindLbl.TextSize=14
local tbKeybindBtn = Instance.new("TextButton",tbc)
tbKeybindBtn.Position=UDim2.new(0,300,0,22)
tbKeybindBtn.Size=UDim2.new(0,70,0,22)
tbKeybindBtn.Text=getgenv().tbKeybind.Name
tbKeybindBtn.Font=Enum.Font.Gotham tbKeybindBtn.TextSize=13
Instance.new("UICorner",tbKeybindBtn).CornerRadius=UDim.new(0,8)
local tbKeybindSet = false
tbKeybindBtn.MouseButton1Click:Connect(function()
    tbKeybindBtn.Text = "[...]"
    tbKeybindSet = true
end)
uis.InputBegan:Connect(function(input,gp)
    if tbKeybindSet and input.UserInputType == Enum.UserInputType.Keyboard then
        if input.KeyCode and input.KeyCode.Name then
            getgenv().tbKeybind = input.KeyCode
            tbKeybindBtn.Text = input.KeyCode.Name
            tbKeybindSet = false
        end
    end
end)

local tbModeLbl=Instance.new("TextLabel",tbc) tbModeLbl.Position=UDim2.new(0,20,0,100) tbModeLbl.Size=UDim2.new(0,55,0,22)
tbModeLbl.Text="Mode:" tbModeLbl.BackgroundTransparency=1 tbModeLbl.TextColor3=COLORS.text tbModeLbl.Font=Enum.Font.Gotham tbModeLbl.TextSize=14
local tbModeSwitch=Instance.new("TextButton",tbc)
tbModeSwitch.Size=UDim2.new(0,85,0,26) tbModeSwitch.Position=UDim2.new(0,80,0,100)
tbModeSwitch.Text=getgenv().tbMode tbModeSwitch.Font=Enum.Font.GothamBold tbModeSwitch.TextSize=14
tbModeSwitch.BackgroundColor3 = Color3.fromRGB(33,143,233)
Instance.new("UICorner",tbModeSwitch).CornerRadius=UDim.new(0,8)
tbModeSwitch.MouseButton1Click:Connect(function()
    if getgenv().tbMode == "Toggle" then getgenv().tbMode = "Hold" else getgenv().tbMode = "Toggle" end
    tbModeSwitch.Text = getgenv().tbMode
end)

local tbHitboxLbl=Instance.new("TextLabel",tbc) tbHitboxLbl.Position=UDim2.new(0,200,0,65) tbHitboxLbl.Size=UDim2.new(0,100,0,19)
tbHitboxLbl.Text="Hit Radius:" tbHitboxLbl.BackgroundTransparency=1 tbHitboxLbl.TextColor3=COLORS.text tbHitboxLbl.Font=Enum.Font.Gotham tbHitboxLbl.TextSize=14
local tbHitboxBox=Instance.new("TextBox",tbc) tbHitboxBox.Size=UDim2.new(0,60,0,26)
tbHitboxBox.Position=UDim2.new(0,300,0,65) tbHitboxBox.Text=tostring(getgenv().hitboxRadius) tbHitboxBox.Font=Enum.Font.Gotham tbHitboxBox.TextSize=14
Instance.new("UICorner",tbHitboxBox).CornerRadius=UDim.new(0,8)
tbHitboxBox.FocusLost:Connect(function()
    local val=tonumber(tbHitboxBox.Text) if val then getgenv().hitboxRadius=val else tbHitboxBox.Text=tostring(getgenv().hitboxRadius) end
end)

local knifeChkLbl=Instance.new("TextLabel",tbc)
knifeChkLbl.Text="Knife Check" knifeChkLbl.Position=UDim2.new(0,20,0,135) knifeChkLbl.Size=UDim2.new(0,80,0,19)
knifeChkLbl.BackgroundTransparency=1 knifeChkLbl.TextColor3=COLORS.text knifeChkLbl.Font=Enum.Font.Gotham knifeChkLbl.TextSize=14
local knifeChkSwitch=Instance.new("TextButton",tbc)
knifeChkSwitch.Size=UDim2.new(0,68,0,22) knifeChkSwitch.Position=UDim2.new(0,110,0,135)
knifeChkSwitch.Text=getgenv().knifeCheck and "On" or "Off"
knifeChkSwitch.Font=Enum.Font.Gotham knifeChkSwitch.TextSize=13
knifeChkSwitch.BackgroundColor3 = getgenv().knifeCheck and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
Instance.new("UICorner",knifeChkSwitch).CornerRadius=UDim.new(0,8)
knifeChkSwitch.MouseButton1Click:Connect(function()
    getgenv().knifeCheck=not getgenv().knifeCheck
    knifeChkSwitch.Text=getgenv().knifeCheck and "On" or "Off"
    knifeChkSwitch.BackgroundColor3=getgenv().knifeCheck and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
end)

-- Notification Toggle for Triggerbot Tab
local tbNotifyLbl=Instance.new("TextLabel",tbc)
tbNotifyLbl.Text="Show Notifications"
tbNotifyLbl.Position=UDim2.new(0,200,0,135+40)
tbNotifyLbl.Size=UDim2.new(0,120,0,19)
tbNotifyLbl.BackgroundTransparency=1
tbNotifyLbl.TextColor3=COLORS.text
tbNotifyLbl.Font=Enum.Font.Gotham tbNotifyLbl.TextSize=14

local tbNotifyToggle=Instance.new("TextButton",tbc)
tbNotifyToggle.Size=UDim2.new(0,68,0,22)
tbNotifyToggle.Position=UDim2.new(0,330,0,135+40)
tbNotifyToggle.Text=getgenv().tbNotify and "On" or "Off"
tbNotifyToggle.Font=Enum.Font.Gotham tbNotifyToggle.TextSize=13
tbNotifyToggle.BackgroundColor3 = getgenv().tbNotify and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
Instance.new("UICorner",tbNotifyToggle).CornerRadius=UDim.new(0,8)
tbNotifyToggle.MouseButton1Click:Connect(function()
    getgenv().tbNotify = not getgenv().tbNotify
    tbNotifyToggle.Text=getgenv().tbNotify and "On" or "Off"
    tbNotifyToggle.BackgroundColor3 = getgenv().tbNotify and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
end)

-- HB Expander Tab
local hbc = tabCons[2]
local hbEnable = Instance.new("TextButton",hbc)
hbEnable.Size=UDim2.new(0,140,0,28)
hbEnable.Position=UDim2.fromOffset(24,28)
hbEnable.Text = getgenv().syrixHitBoxEpanderSettings.Enabled and "HB Expander: ON" or "HB Expander: OFF"
hbEnable.Font=Enum.Font.GothamBold hbEnable.TextSize=14
hbEnable.BackgroundColor3 = getgenv().syrixHitBoxEpanderSettings.Enabled and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
Instance.new("UICorner",hbEnable).CornerRadius=UDim.new(0,8)
hbEnable.MouseButton1Click:Connect(function()
    getgenv().syrixHitBoxEpanderSettings.Enabled = not getgenv().syrixHitBoxEpanderSettings.Enabled
    hbEnable.Text = getgenv().syrixHitBoxEpanderSettings.Enabled and "HB Expander: ON" or "HB Expander: OFF"
    hbEnable.BackgroundColor3 = getgenv().syrixHitBoxEpanderSettings.Enabled and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
end)

local hbSizeLbl=Instance.new("TextLabel",hbc) hbSizeLbl.Position=UDim2.new(0,24,0,68) hbSizeLbl.Size=UDim2.new(0,80,0,19)
hbSizeLbl.Text="HB Size:" hbSizeLbl.BackgroundTransparency=1 hbSizeLbl.TextColor3=COLORS.text hbSizeLbl.Font=Enum.Font.Gotham hbSizeLbl.TextSize=14

local hbSizeBox = Instance.new("TextBox", hbc) hbSizeBox.Size=UDim2.new(0,60,0,26)
hbSizeBox.Position=UDim2.new(0,100,0,68) hbSizeBox.Text = tostring(getgenv().syrixHitBoxEpanderSettings.Size) hbSizeBox.Font=Enum.Font.Gotham hbSizeBox.TextSize=14
Instance.new("UICorner",hbSizeBox).CornerRadius=UDim.new(0,8)
hbSizeBox.FocusLost:Connect(function()
    local val=tonumber(hbSizeBox.Text) if val then getgenv().syrixHitBoxEpanderSettings.Size=val else hbSizeBox.Text=tostring(getgenv().syrixHitBoxEpanderSettings.Size) end
end)

-- Notification Toggle for HB Expander tab
local hbNotifyLbl=Instance.new("TextLabel",hbc)
hbNotifyLbl.Text="Show Notifications"
hbNotifyLbl.Position=UDim2.new(0,24,0,110)
hbNotifyLbl.Size=UDim2.new(0,120,0,19)
hbNotifyLbl.BackgroundTransparency=1
hbNotifyLbl.TextColor3=COLORS.text
hbNotifyLbl.Font=Enum.Font.Gotham hbNotifyLbl.TextSize=14

local hbNotifyToggle=Instance.new("TextButton",hbc)
hbNotifyToggle.Size=UDim2.new(0,68,0,22)
hbNotifyToggle.Position=UDim2.new(0,150,0,110)
hbNotifyToggle.Text=getgenv().hbNotify and "On" or "Off"
hbNotifyToggle.Font=Enum.Font.Gotham hbNotifyToggle.TextSize=13
hbNotifyToggle.BackgroundColor3 = getgenv().hbNotify and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
Instance.new("UICorner",hbNotifyToggle).CornerRadius=UDim.new(0,8)
hbNotifyToggle.MouseButton1Click:Connect(function()
    getgenv().hbNotify = not getgenv().hbNotify
    hbNotifyToggle.Text=getgenv().hbNotify and "On" or "Off"
    hbNotifyToggle.BackgroundColor3 = getgenv().hbNotify and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
end)

-- Walkspeed Tab
local wsc = tabCons[3]
local wsEnable = Instance.new("TextButton",wsc)
wsEnable.Size=UDim2.new(0,110,0,28)
wsEnable.Position=UDim2.fromOffset(24,28)
wsEnable.Text = getgenv().walkSpeedSettings.WalkSpeed.Enabled and "WalkSpeed: ON" or "WalkSpeed: OFF"
wsEnable.Font=Enum.Font.GothamBold wsEnable.TextSize=14
wsEnable.BackgroundColor3 = getgenv().walkSpeedSettings.WalkSpeed.Enabled and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
Instance.new("UICorner",wsEnable).CornerRadius=UDim.new(0,8)
wsEnable.MouseButton1Click:Connect(function()
    getgenv().walkSpeedSettings.WalkSpeed.Enabled = not getgenv().walkSpeedSettings.WalkSpeed.Enabled
    wsEnable.Text = getgenv().walkSpeedSettings.WalkSpeed.Enabled and "WalkSpeed: ON" or "WalkSpeed: OFF"
    wsEnable.BackgroundColor3 = getgenv().walkSpeedSettings.WalkSpeed.Enabled and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
end)

local wsSpeedLbl=Instance.new("TextLabel",wsc) wsSpeedLbl.Position=UDim2.new(0,24,0,70) wsSpeedLbl.Size=UDim2.new(0,70,0,19)
wsSpeedLbl.Text="Speed:" wsSpeedLbl.BackgroundTransparency=1 wsSpeedLbl.TextColor3=COLORS.text wsSpeedLbl.Font=Enum.Font.Gotham wsSpeedLbl.TextSize=14

local wsSpeedBox = Instance.new("TextBox", wsc) wsSpeedBox.Size=UDim2.new(0,60,0,26)
wsSpeedBox.Position=UDim2.new(0,100,0,70) wsSpeedBox.Text = tostring(getgenv().walkSpeedSettings.WalkSpeed.Speed) wsSpeedBox.Font=Enum.Font.Gotham wsSpeedBox.TextSize=14
Instance.new("UICorner",wsSpeedBox).CornerRadius=UDim.new(0,8)
wsSpeedBox.FocusLost:Connect(function()
    local val=tonumber(wsSpeedBox.Text) if val then getgenv().walkSpeedSettings.WalkSpeed.Speed=val else wsSpeedBox.Text=tostring(getgenv().walkSpeedSettings.WalkSpeed.Speed) end
end)

local wsKeybindLbl=Instance.new("TextLabel",wsc) wsKeybindLbl.Position=UDim2.new(0,24,0,110) wsKeybindLbl.Size=UDim2.new(0,100,0,19)
wsKeybindLbl.Text="WalkSpeed Key" wsKeybindLbl.BackgroundTransparency=1 wsKeybindLbl.TextColor3=COLORS.text wsKeybindLbl.Font=Enum.Font.Gotham wsKeybindLbl.TextSize=14

local wsKeybindBtn = Instance.new("TextButton",wsc)
wsKeybindBtn.Position=UDim2.new(0,140,0,110)
wsKeybindBtn.Size=UDim2.new(0,70,0,22)
wsKeybindBtn.Text=getgenv().walkSpeedSettings.Activation.WalkSpeedToggleKey
wsKeybindBtn.Font=Enum.Font.Gotham wsKeybindBtn.TextSize=13
Instance.new("UICorner",wsKeybindBtn).CornerRadius=UDim.new(0,8)
local wsKeybindSet = false
wsKeybindBtn.MouseButton1Click:Connect(function()
    wsKeybindBtn.Text = "[...]"
    wsKeybindSet = true
end)
uis.InputBegan:Connect(function(input,gp)
    if wsKeybindSet and input.UserInputType == Enum.UserInputType.Keyboard then
        if input.KeyCode and input.KeyCode.Name then
            getgenv().walkSpeedSettings.Activation.WalkSpeedToggleKey = input.KeyCode.Name
            wsKeybindBtn.Text = input.KeyCode.Name
            wsKeybindSet = false
        end
    end
end)

-- Notification Toggle for Walkspeed tab
local wsNotifyLbl=Instance.new("TextLabel",wsc)
wsNotifyLbl.Text="Show Notifications"
wsNotifyLbl.Position=UDim2.new(0,24,0,154)
wsNotifyLbl.Size=UDim2.new(0,120,0,19)
wsNotifyLbl.BackgroundTransparency=1
wsNotifyLbl.TextColor3=COLORS.text
wsNotifyLbl.Font=Enum.Font.Gotham wsNotifyLbl.TextSize=14

local wsNotifyToggle=Instance.new("TextButton",wsc)
wsNotifyToggle.Size=UDim2.new(0,68,0,22)
wsNotifyToggle.Position=UDim2.new(0,150,0,154)
wsNotifyToggle.Text=getgenv().wsNotify and "On" or "Off"
wsNotifyToggle.Font=Enum.Font.Gotham wsNotifyToggle.TextSize=13
wsNotifyToggle.BackgroundColor3 = getgenv().wsNotify and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
Instance.new("UICorner",wsNotifyToggle).CornerRadius=UDim.new(0,8)
wsNotifyToggle.MouseButton1Click:Connect(function()
    getgenv().wsNotify = not getgenv().wsNotify
    wsNotifyToggle.Text=getgenv().wsNotify and "On" or "Off"
    wsNotifyToggle.BackgroundColor3 = getgenv().wsNotify and Color3.fromRGB(34,190,50) or Color3.fromRGB(186,60,40)
end)

for i,v in ipairs(tabCons) do v.Parent=main end
main.Parent = screen
tabFrame.Parent = main

-- Insert toggles GUI
local guiVisible = true
uis.InputBegan:Connect(function(input, gp)
    if input.KeyCode == Enum.KeyCode.Insert and not gp then
        guiVisible = not guiVisible
        screen.Enabled = guiVisible
    end
end)



-- start of animation changer /this is only client sided no one else will see the animation u see on yourself\
local Animations = {
    walk = "rbxassetid://619536621",
    run = "rbxassetid://616163682",
    jump = "rbxassetid://10921242013",
    fall = "rbxassetid://10921241244",
    climb = "rbxassetid://507765644",
    swim = "rbxassetid://507784897",
    swimslow = "rbxassetid://507785072",
    toolnone = "rbxassetid://507768375",
    toolslash = "rbxassetid://522635514",
    sit = "rbxassetid://2506281703"
}

local Players = game:GetService("Players")
local lp = Players.LocalPlayer

local function applyAnims(char)
    local function setAnims()
        local hum = char:FindFirstChildOfClass("Humanoid")
        local anims = char:FindFirstChild("Animate")
        if hum and anims then
            if anims:FindFirstChild("walk") and anims.walk:FindFirstChild("WalkAnim") then
                anims.walk.WalkAnim.AnimationId = Animations.walk
            end
            if anims:FindFirstChild("run") and anims.run:FindFirstChild("RunAnim") then
                anims.run.RunAnim.AnimationId = Animations.run
            end
            if anims:FindFirstChild("jump") and anims.jump:FindFirstChild("JumpAnim") then
                anims.jump.JumpAnim.AnimationId = Animations.jump
            end
            if anims:FindFirstChild("fall") and anims.fall:FindFirstChild("FallAnim") then
                anims.fall.FallAnim.AnimationId = Animations.fall
            end
            if anims:FindFirstChild("climb") and anims.climb:FindFirstChild("ClimbAnim") then
                anims.climb.ClimbAnim.AnimationId = Animations.climb
            end
            if anims:FindFirstChild("swim") and anims.swim:FindFirstChild("Swim") then
                anims.swim.Swim.AnimationId = Animations.swim
            end
            if anims:FindFirstChild("swimidle") and anims.swimidle:FindFirstChild("SwimIdle") then
                anims.swimidle.SwimIdle.AnimationId = Animations.swimslow
            end
            if anims:FindFirstChild("toolnone") and anims.toolnone:FindFirstChild("ToolNoneAnim") then
                anims.toolnone.ToolNoneAnim.AnimationId = Animations.toolnone
            end
            if anims:FindFirstChild("toolslash") and anims.toolslash:FindFirstChild("ToolSlashAnim") then
                anims.toolslash.ToolSlashAnim.AnimationId = Animations.toolslash
            end
            if anims:FindFirstChild("sit") and anims.sit:FindFirstChild("SitAnim") then
                anims.sit.SitAnim.AnimationId = Animations.sit
            end
        end
    end

    -- Ensures Apply works after Animate reloads
    local function watchAll()
        setAnims()
        -- Listen for animate script being reinserted/reset
        local animConn
        local function onChildAdded(child)
            if child.Name == "Animate" then
                task.wait(0.1)
                setAnims()
            end
        end
        char.ChildAdded:Connect(onChildAdded)
    end

    -- Run after humanoid & animate script both present
    if not char:FindFirstChild("Humanoid") then char:WaitForChild("Humanoid") end
    if not char:FindFirstChild("Animate") then char:WaitForChild("Animate") end
    task.wait(0.2) -- Give everything a moment
    watchAll()
end

if lp.Character then
    applyAnims(lp.Character)
end

lp.CharacterAdded:Connect(applyAnims)
--end of animation changer

