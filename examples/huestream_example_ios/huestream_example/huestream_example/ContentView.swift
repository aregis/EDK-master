//
//  ContentView.swift
//  huestream_example
//  Copyright © 2021 Signify. All rights reserved.
//

import SwiftUI

struct EntertainmentArea: Identifiable {
	let id: Int
	let name: String
}

class HueHandler: ObservableObject {
	@Published var msg: String = "Searching..."
	@Published var bridgeName: String = ""
	@Published var selectedArea: Int32 = -1 {
		didSet {
			hue.useEntertainmentArea(selectedArea)
		}
	}
	@Published var areaList: Array = [EntertainmentArea(id: 0, name: "")]
	@Published var showPushLink: Bool = false
	@Published var showSearching: Bool = false
	@Published var showMainMenu: Bool = false
	
	private var hue: HueStreamWrapper = HueStreamWrapper()
	//var hue: HueStreamWrapper? // Uncomment if you want to use the preview
	
	init() {
		hue.setHueMessageHandler(HueMessageHandler)
	}
	
	func ShowPushLink() {
		showMainMenu = false
		showSearching = false
		showPushLink = true
	}
	
	func ShowSearching() {
		showMainMenu = false
		showPushLink = false
		showSearching = true
		msg = "Searching..."
		hue.abortConnect()
	}
	
	func ShowMainMenu() {
		showPushLink = false
		showSearching = false
		showMainMenu = true
	}
	
	func ShowWelcome() {
		showMainMenu = false
		showPushLink = false
		showSearching = false
	}
	
	func SetLightsToGreen() {
		hue.setLightsToGreen()
	}
	
	func ShowAnimatedLights() {
		hue.showAnimatedLightExample()
	}
	
	func ResetHueData() {
		hue.resetHueData()
	}
	
	func ConnectToBridge() {
		hue.connect()
	}
	
	func HueMessageHandler(msg:HueStreamFeedbackMessage?) ->Void {
		if (msg != nil && msg?.userMessage != nil) {
			self.msg = msg?.userMessage ?? "Searching...";
		}
		
		if (msg?.feedbackId == ID_PRESS_PUSH_LINK) {
			ShowPushLink()
		}
		else if (msg?.feedbackId == ID_BRIDGE_CONNECTED) {
			bridgeName = hue.getBridgeName() ?? ""
			
			let areaList = hue.getEntertainmentAreaList()
			
			var i = 0
			self.areaList.removeAll()
			while i < areaList!.count {
				let areaName = String(describing: areaList![i])
				let area = EntertainmentArea(id:i, name:areaName)
				self.areaList.append(area)
				i += 1
			}
			
			ShowMainMenu()
		}
		else if (msg?.feedbackId == ID_BRIDGE_DISCONNECTED) {
			ShowWelcome()
		}
		else if (msg?.feedbackId == ID_SELECT_GROUP) {
			// For now always use the first entertainment area when requested by the edk
			selectedArea = 0
		}
		else if (msg?.feedbackId == ID_USERPROCEDURE_FINISHED) {
			selectedArea = hue.getSelectedEntertainmentArea()
		}
	}
}

struct ContentView: View {
	@ObservedObject var hueHandler: HueHandler = HueHandler()
	
	var body: some View {
		return Group {
			if (hueHandler.showMainMenu) {
				MainMenuView(hueHandler: hueHandler)
			}
			else if (hueHandler.showSearching) {
				SearchingView(hueHandler: hueHandler)
			}
			else if (hueHandler.showPushLink) {
				PushLinkView(hueHandler: hueHandler)
			}
			else {
				WelcomeView(hueHandler:hueHandler)
			}
		}
	}
}

struct WelcomeView: View {
	@StateObject var hueHandler: HueHandler
	
	var body: some View {
		VStack {
			Text("Welcome to huestream iOS example")
			Button("Hue Setup") {
				self.hueHandler.ShowSearching()
			}
			.padding(.all, 5.0)
			.background(Color.gray.cornerRadius(8.0))
			.foregroundColor(/*@START_MENU_TOKEN@*/.white/*@END_MENU_TOKEN@*/)
		}
	}
}

struct SearchingView: View {
	@StateObject var hueHandler: HueHandler
	
	var body: some View {
		return VStack {
			Text(hueHandler.msg)
		}
		.onAppear() {
			DispatchQueue.main.async {
				hueHandler.ConnectToBridge()
			}
		}
	}
}

struct PushLinkView: View {
	@StateObject var hueHandler: HueHandler
	@State var durationLeft: Int = 30
	
	let timer = Timer.publish(every: 1, on: .main, in: .common).autoconnect()
	
	var body: some View {
		VStack {
			Text("Please press the push-link button on your Hue Bridge")
			Text("\(durationLeft) sec")
				.onReceive(timer, perform: { _ in
					if (durationLeft > 0) {
						durationLeft -= 1
					}
					else {
						hueHandler.ShowSearching()
					}
			})
		}
	}
}

struct MainMenuView: View {
	@StateObject var hueHandler: HueHandler
	@State private var selectedArea = -1
	var body: some View {
		VStack {
			Text(hueHandler.selectedArea  < 0 ? "Syncing with \(hueHandler.bridgeName)" : "Syncing with \(hueHandler.bridgeName) on \(hueHandler.areaList[Int(hueHandler.selectedArea)].name)")
				.multilineTextAlignment(.center)
				.padding(.bottom, 10.0)
			Picker("Select area", selection: $selectedArea/*$hueHandler.selectedArea*/) {
				ForEach(hueHandler.areaList) { area in
					Text(area.name)
				}
			}
			.pickerStyle(MenuPickerStyle())
			.onAppear() {selectedArea = Int(hueHandler.selectedArea)}
			.onChange(of: selectedArea, perform: { value in
				hueHandler.selectedArea = Int32(selectedArea)
			})
			
			Button("Reset hue data") {
				self.hueHandler.ResetHueData()
			}
			.padding(.all, 5.0)
			.background(Color.gray.cornerRadius(8.0))
			.foregroundColor(/*@START_MENU_TOKEN@*/.white/*@END_MENU_TOKEN@*/)
			
			Button("Set lights to green") {
				self.hueHandler.SetLightsToGreen()
			}
			.padding(.all, 5.0)
			.background(Color.gray.cornerRadius(8.0))
			.foregroundColor(/*@START_MENU_TOKEN@*/.white/*@END_MENU_TOKEN@*/)
			
			Button("Show animated lights") {
				self.hueHandler.ShowAnimatedLights()
			}
			.padding(.all, 5.0)
			.background(Color.gray.cornerRadius(8.0))
			.foregroundColor(/*@START_MENU_TOKEN@*/.white/*@END_MENU_TOKEN@*/)
		}
	}
}

struct ContentView_Previews: PreviewProvider {
	static var previews: some View {
		ContentView()
	}
}
