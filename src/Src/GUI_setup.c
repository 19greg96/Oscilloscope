
#include <MonoGUI_font_legible3x5_6pt.h>
#include "GUI_setup.h"
#include "BSP_UIIO.h"
#include "bode.h"

// TODO: rename components: [screen][Name][Type] example: ssTrigLevelLabel
// Main Screen
MonoGUI_Component* ms1_scopeButton;
MonoGUI_Component* ms2_fnGenButton;
MonoGUI_Component* ms3_bodeButton;
MonoGUI_Component* ms4_backlightScrollButton;
MonoGUI_Component* bottomMenuOutlineSpriteComponent;

// Scope Screen
MonoGUI_Graph* scopeGraph;

MonoGUI_Component* ss1_runStopToggleButtonComponent;
MonoGUI_Component* ss4_cfgMenuButtonComponent;

MonoGUI_GraphLabel* trigLevelLabel;
MonoGUI_Component* trigLevelLabelComponent;
MonoGUI_GraphLabel* zeroLevelALabel;
MonoGUI_GraphLabel* zeroLevelBLabel;
MonoGUI_GraphLabel* hScrollLabel;
MonoGUI_Component* hScrollLabelComponent;

MonoGUI_RadioButton* trigModeNormalRadioButton;
MonoGUI_RadioButton* trigModeSingleRadioButton;
MonoGUI_RadioButton* trigModeDisableRadioButton;

MonoGUI_RadioButton* scopeModeNormalRadioButton;
MonoGUI_RadioButton* scopeModeXYRadioButton;
MonoGUI_RadioButton* scopeModeFFT1RadioButton;
MonoGUI_RadioButton* scopeModeFFT2RadioButton;

// Function generator Screen
MonoGUI_Component* fs1_fnMenuButton;
MonoGUI_Component* fs2_prevButton;
MonoGUI_Component* fs3_nextButton;
MonoGUI_ToggleButton* fs4_channelToggleButton;
MonoGUI_Component* fs4_channelToggleButtonComponent;

MonoGUI_Label* fs_channelLabel;
MonoGUI_Range* frequencyRange;
MonoGUI_Range* maxRange;
MonoGUI_Range* minRange;
MonoGUI_Range* riseRange;
MonoGUI_Range* dutyRange;
MonoGUI_Range* shapeRange;

MonoGUI_Component* fs_channelLabelComponent;
MonoGUI_Component* frequencyRangeComponent;
MonoGUI_Component* maxRangeComponent;
MonoGUI_Component* minRangeComponent;
MonoGUI_Component* dutyRangeComponent;
MonoGUI_Component* riseRangeComponent;
MonoGUI_Component* shapeRangeComponent;

MonoGUI_RadioButton* fnSineRadioButton;
MonoGUI_RadioButton* fnSquareRadioButton;
MonoGUI_RadioButton* fnTriRadioButton;

MonoGUI_ToggleButton* outputEnableToggleButton;
MonoGUI_ToggleButton* outputBufferToggleButton;
MonoGUI_Component* outputEnableToggleButtonComponent;
MonoGUI_Component* outputBufferToggleButtonComponent;

// Bode Screen
MonoGUI_Component* bs1_measureButton;
MonoGUI_ScrollButton* bs2_fminScrollButton;
MonoGUI_Component* bs2_fminScrollButtonComponent;
MonoGUI_ScrollButton* bs3_fmaxScrollButton;
MonoGUI_Component* bs3_fmaxScrollButtonComponent;
MonoGUI_Component* bs4_toggleButton;


// TODO: rename event handlers: on[Event][ComponentName][ComponentType] example: onClickSoftBtn1

// general event handlers
void onSoftBtn1Click(UIIO_BtnTypedef* btn);
void onSoftBtn2Click(UIIO_BtnTypedef* btn);
void onSoftBtn3Click(UIIO_BtnTypedef* btn);
void onSoftBtn4Click(UIIO_BtnTypedef* btn);
void onMainLongPress(UIIO_BtnTypedef* mainBtn);
void onMainClick(UIIO_BtnTypedef* mainBtn);
void onScroll(int16_t delta);

// main screen
void onScopeBtnClick(void* caller);
void onFnGenBtnClick(void* caller);
void onBodeBtnClick(void* caller);
void onBrightnessScroll(void* caller);
void onDrawEncoderDebugSprite(MonoGUI_Sprite* sprite, int32_t x, int32_t y);

// scope screen
void on1ACToggle(void* caller);
void on1AAToggle(void* caller);
void on2ACToggle(void* caller);
void on2AAToggle(void* caller);

void onModeSelect(void* caller);
void updateScopeTriggerMode();
void onTriggerToggle(void* caller);
void onTriggerModeSelect(void* caller);
void onRunStopToggle(void* caller);
void onScopeStop();
void onTrigScroll(void* caller);
void onHDivScroll(void* caller);

// fngen screen
void drawFunctionSprite(MonoGUI_Sprite* sprite, int32_t x, int32_t y);
void setDACConfig(void* caller);
void onChannelSelect(void* caller);
void onFunctionSelect(void* caller);
void onFreqScroll(void* caller);
void onMinScroll(void* caller);
void onMaxScroll(void* caller);
void onDutyScroll(void* caller);
void onRiseScroll(void* caller);
void onShapeScroll(void* caller);
void onNextBtn(void* caller);
void onPrevBtn(void* caller);

// bode screen
void onBodeMeasureClick(void* caller);
void onDrawBodeProgressSprite(MonoGUI_Sprite* sprite, int32_t x, int32_t y);

void onDrawBottomMenuOutlineSprite(MonoGUI_Sprite* sprite, int32_t x, int32_t y);

uint32_t defaultFont;

void mainScreenSetup() { // main screen
	MonoGUI_Screen* functionSelectScreen = MonoGUI_screen_create();
	MonoGUI_screen_add_component(functionSelectScreen, bottomMenuOutlineSpriteComponent);
	/*
	MonoGUI_Sprite* encoderDebugSprite = MonoGUI_sprite_create(onDrawEncoderDebugSprite);
	MonoGUI_Component* encoderDebugSpriteComponent = MonoGUI_component_create(MonoGUI_COMPONENT_SPRITE, 0, 0, encoderDebugSprite);
	MonoGUI_screen_add_component(functionSelectScreen, encoderDebugSpriteComponent);
	*/
	MonoGUI_ScrollButton* sb = MonoGUI_scrollButton_create("Light", defaultFont, 0.0f, 100.0f, 50.0f, onBrightnessScroll);
	sb->smallStep = 1.0f;
	sb->largeStep = 5.0f;
	sb->panelEnabled = 1;
	
	ms1_scopeButton			= MonoGUI_component_create(MonoGUI_COMPONENT_BUTTON,		2,							MonoGFXDisplay.height - LINE_HEIGHT, MonoGUI_button_create("Scope", defaultFont, onScopeBtnClick));
	ms2_fnGenButton			= MonoGUI_component_create(MonoGUI_COMPONENT_BUTTON,		MonoGFXDisplay.width / 4 + 2,			MonoGFXDisplay.height - LINE_HEIGHT, MonoGUI_button_create("Fn gen", defaultFont, onFnGenBtnClick));
	ms3_bodeButton			= MonoGUI_component_create(MonoGUI_COMPONENT_BUTTON,		MonoGFXDisplay.width / 4 * 2 + 2,		MonoGFXDisplay.height - LINE_HEIGHT, MonoGUI_button_create("Bode", defaultFont, onBodeBtnClick));
	ms4_backlightScrollButton= MonoGUI_component_create(MonoGUI_COMPONENT_SCROLL_BUTTON,MonoGFXDisplay.width / 4 * 3 + 2,		MonoGFXDisplay.height - LINE_HEIGHT, sb);
	
	MonoGUI_scrollButton_scroll(sb, 0, 0); // DEFAULT CONFIG
	
	MonoGUI_screen_add_component(functionSelectScreen, ms1_scopeButton);
	MonoGUI_screen_add_component(functionSelectScreen, ms2_fnGenButton);
	MonoGUI_screen_add_component(functionSelectScreen, ms3_bodeButton);
	MonoGUI_screen_add_component(functionSelectScreen, ms4_backlightScrollButton);
}

void scopeScreenSetup() { // scope screen
	MonoGUI_Screen* scopeScreen = MonoGUI_screen_create();
	
	scopeGraph = MonoGUI_graph_create(MonoGFXDisplay.width, MonoGFXDisplay.height - 9);
	scopeGraph->data_1 = g_graphBuffer1_V;
	scopeGraph->data_2 = g_graphBuffer2_V;
	scopeGraph->dataBufferSize = ADC_INPUT_BUFFER_SIZE / 2;
	scopeGraph->mode = MonoGUI_GRAPH_MODE_12CH_NORMAL;
	arm_rfft_fast_init_f32(&scopeGraph->FFTStruct, scopeGraph->dataBufferSize); // this graph will be able to run FFT (TODO: this should be done in graph or in global math handler)
	
	MonoGUI_Component* scopeGraphComponent = MonoGUI_component_create(MonoGUI_COMPONENT_GRAPH, 0, 0, scopeGraph);
	
	zeroLevelALabel = MonoGUI_graphLabel_create("A0", MonoGFX_LINE_STYLE_SPARSE_DOTS, 0, 1, defaultFont, NULL);
	MonoGUI_Component* zeroLevelALabelComponent = MonoGUI_component_create(MonoGUI_COMPONENT_GRAPH_LABEL, 0, 0, zeroLevelALabel);
	
	zeroLevelALabel->nextLabel = zeroLevelALabelComponent;
	scopeGraph->vOffset1GraphLabel = zeroLevelALabel;
	MonoGUI_graph_add_label(scopeGraph, zeroLevelALabelComponent);
	
	zeroLevelBLabel = MonoGUI_graphLabel_create("B0", MonoGFX_LINE_STYLE_SPARSE_DOTS, 0, 1, defaultFont, NULL); // TODO: B channel zero level should only be visible when B channel is enabled
	zeroLevelBLabel->rightChannel = 1;
	MonoGUI_Component* zeroLevelBLabelComponent = MonoGUI_component_create(MonoGUI_COMPONENT_GRAPH_LABEL, 0, 0, zeroLevelBLabel);
	zeroLevelBLabel->nextLabel = zeroLevelBLabelComponent;
	scopeGraph->vOffset2GraphLabel = zeroLevelBLabel;
	MonoGUI_graph_add_label(scopeGraph, zeroLevelBLabelComponent);
	
	trigLevelLabel = MonoGUI_graphLabel_create("TA~", MonoGFX_LINE_STYLE_SPARSE_DOTS, 0, 1, defaultFont, onTrigScroll);
	trigLevelLabelComponent = MonoGUI_component_create(MonoGUI_COMPONENT_GRAPH_LABEL, 0, 0, trigLevelLabel);
	trigLevelLabel->scrollButton->button->onClick = onTriggerToggle;
	trigLevelLabel->nextLabel = trigLevelLabelComponent;
	MonoGUI_graph_add_label(scopeGraph, trigLevelLabelComponent);
	MonoGUI_scrollButton_scroll(trigLevelLabel->scrollButton, 0, 0); // DEFAULT CONFIG: set trigger level
	
	hScrollLabel = MonoGUI_graphLabel_create("", MonoGFX_LINE_STYLE_SPARSE_DOTS, 0, 0, defaultFont, NULL);
	hScrollLabelComponent = MonoGUI_component_create(MonoGUI_COMPONENT_GRAPH_LABEL, 0, 0, hScrollLabel);
	hScrollLabel->nextLabel = hScrollLabelComponent;
	scopeGraph->hOffsetGraphLabel = hScrollLabel;
	MonoGUI_graph_add_label(scopeGraph, hScrollLabelComponent);
	
	scopeScreen->defaultTabComponent = scopeGraph->vDiv1ScrollButtonComponent;
	MonoGUI_component_set_tabNext(scopeGraph->vDiv1ScrollButtonComponent, scopeGraph->hDivScrollButtonComponent);
	MonoGUI_component_set_tabNext(scopeGraph->hDivScrollButtonComponent, scopeGraph->vDiv2ScrollButtonComponent);
	MonoGUI_component_set_tabNext(scopeGraph->vDiv2ScrollButtonComponent, trigLevelLabelComponent);
	MonoGUI_component_set_tabNext(trigLevelLabelComponent, zeroLevelALabelComponent);
	MonoGUI_component_set_tabNext(zeroLevelALabelComponent, zeroLevelBLabelComponent);
	MonoGUI_component_set_tabNext(zeroLevelBLabelComponent, hScrollLabelComponent);
	MonoGUI_component_set_tabNext(hScrollLabelComponent, scopeGraph->vDiv1ScrollButtonComponent);
	
	MonoGUI_screen_add_component(scopeScreen, scopeGraphComponent);
	
	MonoGUI_screen_add_component(scopeScreen, bottomMenuOutlineSpriteComponent);
	
	zeroLevelALabel->scrollButton->min = -5.0f;
	zeroLevelALabel->scrollButton->max = 5.0f;
	zeroLevelALabel->scrollButton->value = 2.0f;
	zeroLevelALabel->scrollButton->largeStep = 0.1f;
	zeroLevelALabel->scrollButton->smallStep = 0.01f;
	
	zeroLevelBLabel->scrollButton->min = -5.0f;
	zeroLevelBLabel->scrollButton->max = 5.0f;
	zeroLevelBLabel->scrollButton->value = 0.0f;
	zeroLevelBLabel->scrollButton->largeStep = 0.1f;
	zeroLevelBLabel->scrollButton->smallStep = 0.01f;
	
	trigLevelLabel->scrollButton->min = -2.5f;
	trigLevelLabel->scrollButton->max = 2.5f;
	trigLevelLabel->scrollButton->value = 0.0f;
	trigLevelLabel->scrollButton->largeStep = 0.1f;
	trigLevelLabel->scrollButton->smallStep = 0.01f;
	
	hScrollLabel->scrollButton->min = -((float)scopeGraph->dataBufferSize - (float)scopeGraph->w) / 2.0f; // TODO: these should be calculated by graph when label is added to it
	hScrollLabel->scrollButton->max = ((float)scopeGraph->dataBufferSize - (float)scopeGraph->w) / 2.0f;
	hScrollLabel->scrollButton->value = 0.0f;
	hScrollLabel->scrollButton->largeStep = -10.0f; // inverted direction
	hScrollLabel->scrollButton->smallStep = -1.0f;
	
	((MonoGUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->min = 0.0f;
	((MonoGUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->max = (float)(numADCFrequencySettings - 1);
	((MonoGUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->value = 0.0f;
	((MonoGUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->largeStep = 2.0f;
	((MonoGUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->smallStep = 1.0f;
	((MonoGUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->onScroll = onHDivScroll;
	((MonoGUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->labelIsValue = 0;
	MonoGUI_scrollButton_scroll(((MonoGUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component), 0, 0); // DEFAULT CONFIG: set horizontal div
	
	((MonoGUI_ScrollButton*)scopeGraph->vDiv1ScrollButtonComponent->component)->min = 0.25f;
	((MonoGUI_ScrollButton*)scopeGraph->vDiv1ScrollButtonComponent->component)->max = 2.0f;
	((MonoGUI_ScrollButton*)scopeGraph->vDiv1ScrollButtonComponent->component)->value = 1.0f;
	((MonoGUI_ScrollButton*)scopeGraph->vDiv1ScrollButtonComponent->component)->largeStep = 0.1f;
	((MonoGUI_ScrollButton*)scopeGraph->vDiv1ScrollButtonComponent->component)->smallStep = 0.01f;
	
	((MonoGUI_ScrollButton*)scopeGraph->vDiv2ScrollButtonComponent->component)->min = 0.25f;
	((MonoGUI_ScrollButton*)scopeGraph->vDiv2ScrollButtonComponent->component)->max = 2.0f;
	((MonoGUI_ScrollButton*)scopeGraph->vDiv2ScrollButtonComponent->component)->value = 1.0f;
	((MonoGUI_ScrollButton*)scopeGraph->vDiv2ScrollButtonComponent->component)->largeStep = 0.1f;
	((MonoGUI_ScrollButton*)scopeGraph->vDiv2ScrollButtonComponent->component)->smallStep = 0.01f;
	
	MonoGUI_ToggleButton* ss1_runStopToggleButton = MonoGUI_toggleButton_create("   RUN", defaultFont, onRunStopToggle);
	ss1_runStopToggleButton->showCheckbox = 0;
	strcpy(ss1_runStopToggleButton->checkedText, "  STOP");
	ss1_runStopToggleButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_TOGGLE_BUTTON, 2, MonoGFXDisplay.height - LINE_HEIGHT, ss1_runStopToggleButton);
	MonoGUI_toggleButton_click(ss1_runStopToggleButton); // DEFAULT CONFIG: run
	
	
	MonoGUI_Menu* cfgMenu = MonoGUI_menu_create();
	
	MonoGUI_MenuColumn* trigSourceMenuColumn = MonoGUI_menuColumn_create();
	trigModeNormalRadioButton = MonoGUI_radioButton_create("T Norm", defaultFont, onTriggerModeSelect);
	trigModeSingleRadioButton = MonoGUI_radioButton_create("T Once", defaultFont, onTriggerModeSelect);
	trigModeDisableRadioButton = MonoGUI_radioButton_create("T Off", defaultFont, onTriggerModeSelect);
	MonoGUI_Component* trigModeNormalRadioButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RADIO_BUTTON, 0, 0, trigModeNormalRadioButton);
	MonoGUI_Component* trigModeSingleRadioButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RADIO_BUTTON, 0, 0, trigModeSingleRadioButton);
	MonoGUI_Component* trigModeDisableRadioButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RADIO_BUTTON, 0, 0, trigModeDisableRadioButton);
	MonoGUI_menuColumn_add_component(trigSourceMenuColumn, trigModeNormalRadioButtonComponent);
	MonoGUI_menuColumn_add_component(trigSourceMenuColumn, trigModeSingleRadioButtonComponent);
	MonoGUI_menuColumn_add_component(trigSourceMenuColumn, trigModeDisableRadioButtonComponent);
	
	MonoGUI_radioButton_click(trigModeDisableRadioButton); // DEFAULT CONFIG: select trigger off mode
	
	MonoGUI_MenuColumn* scopeModeMenuColumn = MonoGUI_menuColumn_create();
	scopeModeNormalRadioButton = MonoGUI_radioButton_create("Normal", defaultFont, onModeSelect);
	scopeModeXYRadioButton = MonoGUI_radioButton_create("XY", defaultFont, onModeSelect);
	scopeModeFFT1RadioButton = MonoGUI_radioButton_create("FFT 1", defaultFont, onModeSelect);
	scopeModeFFT2RadioButton = MonoGUI_radioButton_create("FFT 2", defaultFont, onModeSelect);
	MonoGUI_Component* scopeModeNormalRadioButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RADIO_BUTTON, 0, 0, scopeModeNormalRadioButton);
	MonoGUI_Component* scopeModeXYRadioButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RADIO_BUTTON, 0, 0, scopeModeXYRadioButton);
	MonoGUI_Component* scopeModeFFT1RadioButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RADIO_BUTTON, 0, 0, scopeModeFFT1RadioButton);
	MonoGUI_Component* scopeModeFFT2RadioButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RADIO_BUTTON, 0, 0, scopeModeFFT2RadioButton);
	MonoGUI_menuColumn_add_component(scopeModeMenuColumn, scopeModeNormalRadioButtonComponent);
	MonoGUI_menuColumn_add_component(scopeModeMenuColumn, scopeModeXYRadioButtonComponent);
	MonoGUI_menuColumn_add_component(scopeModeMenuColumn, scopeModeFFT1RadioButtonComponent);
	MonoGUI_menuColumn_add_component(scopeModeMenuColumn, scopeModeFFT2RadioButtonComponent);
	
	MonoGUI_radioButton_click(scopeModeNormalRadioButton); // DEFAULT CONFIG: select normal scope mode
	
	MonoGUI_MenuColumn* inputFilterMenuColumn = MonoGUI_menuColumn_create();
	MonoGUI_ToggleButton* inputFilter1ACToggleButton = MonoGUI_toggleButton_create("1AC", defaultFont, on1ACToggle);
	MonoGUI_ToggleButton* inputFilter1AAToggleButton = MonoGUI_toggleButton_create("1AA", defaultFont, on1AAToggle);
	MonoGUI_ToggleButton* inputFilter2ACToggleButton = MonoGUI_toggleButton_create("2AC", defaultFont, on2ACToggle);
	MonoGUI_ToggleButton* inputFilter2AAToggleButton = MonoGUI_toggleButton_create("2AA", defaultFont, on2AAToggle);
	
	MonoGUI_toggleButton_click(inputFilter1ACToggleButton); // DEFAULT CONFIG: relay configuration
	MonoGUI_toggleButton_click(inputFilter1AAToggleButton); // DEFAULT CONFIG: relay configuration
	MonoGUI_toggleButton_click(inputFilter2ACToggleButton); // DEFAULT CONFIG: relay configuration
	MonoGUI_toggleButton_click(inputFilter2AAToggleButton); // DEFAULT CONFIG: relay configuration
	
	MonoGUI_Component* inputFilter1ACToggleButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_TOGGLE_BUTTON, 0, 0, inputFilter1ACToggleButton);
	MonoGUI_Component* inputFilter1AAToggleButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_TOGGLE_BUTTON, 0, 0, inputFilter1AAToggleButton);
	MonoGUI_Component* inputFilter2ACToggleButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_TOGGLE_BUTTON, 0, 0, inputFilter2ACToggleButton);
	MonoGUI_Component* inputFilter2AAToggleButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_TOGGLE_BUTTON, 0, 0, inputFilter2AAToggleButton);
	MonoGUI_menuColumn_add_component(inputFilterMenuColumn, inputFilter1ACToggleButtonComponent);
	MonoGUI_menuColumn_add_component(inputFilterMenuColumn, inputFilter1AAToggleButtonComponent);
	MonoGUI_menuColumn_add_component(inputFilterMenuColumn, inputFilter2ACToggleButtonComponent);
	MonoGUI_menuColumn_add_component(inputFilterMenuColumn, inputFilter2AAToggleButtonComponent);
	
	MonoGUI_menu_add_column(cfgMenu, trigSourceMenuColumn);
	MonoGUI_menu_add_column(cfgMenu, scopeModeMenuColumn);
	MonoGUI_menu_add_column(cfgMenu, inputFilterMenuColumn);
	ss4_cfgMenuButtonComponent		= MonoGUI_component_create(MonoGUI_COMPONENT_MENU_BUTTON,	MonoGFXDisplay.width / 4 * 3 + 2,		MonoGFXDisplay.height - LINE_HEIGHT, MonoGUI_menuButton_create("Cfg", defaultFont, NULL, cfgMenu));
	MonoGUI_screen_add_component(scopeScreen, ss1_runStopToggleButtonComponent);
	// MonoGUI_screen_add_component(scopeScreen, ss2_vdivScrollButtonComponent);
	MonoGUI_screen_add_component(scopeScreen, fs2_prevButton);
	MonoGUI_screen_add_component(scopeScreen, fs3_nextButton);
	// MonoGUI_screen_add_component(scopeScreen, ss3_trigScrollButtonComponent);
	MonoGUI_screen_add_component(scopeScreen, ss4_cfgMenuButtonComponent);
	
	SCOPE_handleOnStop = onScopeStop;
}

void functionGeneratorScreenSetup() { // Fn gen screen
	MonoGUI_Screen* fnGenScreen = MonoGUI_screen_create();
	
	MonoGUI_screen_add_component(fnGenScreen, bottomMenuOutlineSpriteComponent);
	
	MonoGUI_Sprite* fnSprite = MonoGUI_sprite_create(drawFunctionSprite);
	MonoGUI_Component* fnSpriteComponent = MonoGUI_component_create(MonoGUI_COMPONENT_SPRITE, 44, 1, fnSprite);
	MonoGUI_screen_add_component(fnGenScreen, fnSpriteComponent);
	// Setting buttons
	fs_channelLabel = MonoGUI_label_create("CH 1", defaultFont);
	fs_channelLabelComponent = MonoGUI_component_create(MonoGUI_COMPONENT_LABEL, 102, 32, fs_channelLabel);
	MonoGUI_screen_add_component(fnGenScreen, fs_channelLabelComponent);
	outputEnableToggleButton = MonoGUI_toggleButton_create("Output", defaultFont, setDACConfig);
	outputBufferToggleButton = MonoGUI_toggleButton_create("Buffer", defaultFont, setDACConfig);
	outputEnableToggleButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_TOGGLE_BUTTON, 102, 40, outputEnableToggleButton);
	outputBufferToggleButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_TOGGLE_BUTTON, 102, 48, outputBufferToggleButton);
	MonoGUI_screen_add_component(fnGenScreen, outputEnableToggleButtonComponent);
	MonoGUI_screen_add_component(fnGenScreen, outputBufferToggleButtonComponent);
	
	// outputBufferToggleButton->checked = 1; // DEFAULT CONFIG: buffer enabled
	
	frequencyRange = MonoGUI_range_create("1 Freq", defaultFont, 1.0f, 1000000.1f, 1000.0f, onFreqScroll, 38, formatFrequency);
	frequencyRangeComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RANGE, 1, 38, frequencyRange);
	MonoGUI_screen_add_component(fnGenScreen, frequencyRangeComponent);
	fnGenScreen->defaultTabComponent = frequencyRangeComponent;
	// Sine
	minRange = MonoGUI_range_create("2 Min", defaultFont, 0.0f, 3.3f, 0.3f, onMinScroll, 29, formatVoltage);
	minRange->vertical = 1;
	minRange->scrollButton->button->label->align = MonoGUI_TEXT_ALIGN_RIGHT;
	minRange->scrollButton->smallStep = 0.01f;
	minRange->scrollButton->largeStep = 0.1f;
	minRange->valueLabel->align = MonoGUI_TEXT_ALIGN_RIGHT;
	minRangeComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RANGE, 41, 1, minRange);
	MonoGUI_screen_add_component(fnGenScreen, minRangeComponent);
	
	maxRange = MonoGUI_range_create("3 Max", defaultFont, 0.0f, 3.3f, 3.0f, onMaxScroll, 29, formatVoltage);
	maxRange->vertical = 1;
	maxRange->scrollButton->smallStep = 0.01f;
	maxRange->scrollButton->largeStep = 0.1f;
	maxRangeComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RANGE, 101, 1, maxRange);
	MonoGUI_screen_add_component(fnGenScreen, maxRangeComponent);
	// Square
	// TODO: update step sizes based on DAC buffer size (minimum resolution)
	dutyRange = MonoGUI_range_create("4 Duty", defaultFont, 0.0f, 100.0f, 50.0f, onDutyScroll, 54, formatPercentage);
	dutyRange->scrollButton->smallStep = 0.1f;
	dutyRange->scrollButton->largeStep = 10.0f;
	dutyRange->valueLabel->align = MonoGUI_TEXT_ALIGN_RIGHT;
	dutyRangeComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RANGE, 44, 33, dutyRange);
	MonoGUI_screen_add_component(fnGenScreen, dutyRangeComponent);
	
	riseRange = MonoGUI_range_create("5 Rise", defaultFont, 0.0f, 100.0f, 0.0f, onRiseScroll, 54, formatPercentage);
	riseRange->scrollButton->smallStep = 1.0f;
	riseRange->scrollButton->largeStep = 10.0f;
	riseRange->valueLabel->align = MonoGUI_TEXT_ALIGN_RIGHT;
	riseRangeComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RANGE, 44, 45, riseRange);
	MonoGUI_screen_add_component(fnGenScreen, riseRangeComponent);
	// Triangle
	shapeRange = MonoGUI_range_create("4 Shape", defaultFont, 0.0f, 100.0f, 50.0f, onShapeScroll, 54, formatPercentage);
	shapeRange->scrollButton->smallStep = 1.0f;
	shapeRange->scrollButton->largeStep = 10.0f;
	shapeRange->valueLabel->align = MonoGUI_TEXT_ALIGN_RIGHT;
	shapeRangeComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RANGE, 44, 33, shapeRange);
	MonoGUI_screen_add_component(fnGenScreen, shapeRangeComponent);
	
	MonoGUI_Menu* fnMenu = MonoGUI_menu_create();
	
	MonoGUI_MenuColumn* fnMenuColumn = MonoGUI_menuColumn_create();
	fnSineRadioButton = MonoGUI_radioButton_create("Sine", defaultFont, onFunctionSelect);
	MonoGUI_Component* fnSineRadioButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RADIO_BUTTON, 0, 0, fnSineRadioButton);
	fnSquareRadioButton = MonoGUI_radioButton_create("Square", defaultFont, onFunctionSelect);
	MonoGUI_Component* fnSquareRadioButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RADIO_BUTTON, 0, 0, fnSquareRadioButton);
	fnTriRadioButton = MonoGUI_radioButton_create("Tri", defaultFont, onFunctionSelect);
	MonoGUI_Component* fnTriRadioButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_RADIO_BUTTON, 0, 0, fnTriRadioButton);
	MonoGUI_menuColumn_add_component(fnMenuColumn, fnSineRadioButtonComponent);
	MonoGUI_menuColumn_add_component(fnMenuColumn, fnSquareRadioButtonComponent);
	MonoGUI_menuColumn_add_component(fnMenuColumn, fnTriRadioButtonComponent);
	
	MonoGUI_menu_add_column(fnMenu, fnMenuColumn);
	MonoGUI_radioButton_click(fnSineRadioButton); // DEFAULT CONFIG: select sine
	
	fs1_fnMenuButton		= MonoGUI_component_create(MonoGUI_COMPONENT_MENU_BUTTON,	2,						MonoGFXDisplay.height - LINE_HEIGHT, MonoGUI_menuButton_create("Fn", defaultFont, NULL, fnMenu));
	
	fs4_channelToggleButton = MonoGUI_toggleButton_create("Toggle", defaultFont, onChannelSelect);
	fs4_channelToggleButton->showCheckbox = 0;
	fs4_channelToggleButtonComponent = MonoGUI_component_create(MonoGUI_COMPONENT_TOGGLE_BUTTON, MonoGFXDisplay.width / 4 * 3 + 2,	MonoGFXDisplay.height - LINE_HEIGHT, fs4_channelToggleButton);
	MonoGUI_screen_add_component(fnGenScreen, fs1_fnMenuButton);
	MonoGUI_screen_add_component(fnGenScreen, fs2_prevButton);
	MonoGUI_screen_add_component(fnGenScreen, fs3_nextButton);
	MonoGUI_screen_add_component(fnGenScreen, fs4_channelToggleButtonComponent);
}

void bodeScreenSetup() { // Bode screen
	MonoGUI_Screen* bodeScreen = MonoGUI_screen_create();
	MonoGUI_screen_add_component(bodeScreen, bottomMenuOutlineSpriteComponent);
	
	bs1_measureButton		= MonoGUI_component_create(MonoGUI_COMPONENT_BUTTON,		2,						MonoGFXDisplay.height - LINE_HEIGHT, MonoGUI_button_create("Measure", defaultFont, onBodeMeasureClick));
	bs2_fminScrollButton = MonoGUI_scrollButton_create("Fmin", defaultFont, 0, BODE_NUM_CONFIGURATIONS, 0, NULL);
	bs2_fminScrollButton->panelEnabled = 1;
	bs2_fminScrollButtonComponent	= MonoGUI_component_create(MonoGUI_COMPONENT_SCROLL_BUTTON,	MonoGFXDisplay.width / 4 + 2,		MonoGFXDisplay.height - LINE_HEIGHT, bs2_fminScrollButton);
	bs3_fmaxScrollButton = MonoGUI_scrollButton_create("Fmax", defaultFont, 0, 10, 1, NULL);
	bs3_fmaxScrollButton->panelEnabled = 1;
	bs3_fmaxScrollButtonComponent	= MonoGUI_component_create(MonoGUI_COMPONENT_SCROLL_BUTTON,	MonoGFXDisplay.width / 4 * 2 + 2,	MonoGFXDisplay.height - LINE_HEIGHT, bs3_fmaxScrollButton);
	bs4_toggleButton		= MonoGUI_component_create(MonoGUI_COMPONENT_BUTTON,		MonoGFXDisplay.width / 4 * 3 + 2,	MonoGFXDisplay.height - LINE_HEIGHT, MonoGUI_button_create("Toggle", defaultFont, NULL));
	
	MonoGUI_Sprite* bsProgressSprite = MonoGUI_sprite_create(onDrawBodeProgressSprite);
	MonoGUI_Component* bsProgressSpriteComponent = MonoGUI_component_create(MonoGUI_COMPONENT_SPRITE, 32, 10, bsProgressSprite);
	MonoGUI_screen_add_component(bodeScreen, bsProgressSpriteComponent);
	
	MonoGUI_screen_add_component(bodeScreen, bs1_measureButton);
	MonoGUI_screen_add_component(bodeScreen, bs2_fminScrollButtonComponent);
	MonoGUI_screen_add_component(bodeScreen, bs3_fmaxScrollButtonComponent);
	MonoGUI_screen_add_component(bodeScreen, bs4_toggleButton);
	
	// TODO: bode graph
}

void GUI_setup_init() {
	defaultFont = MonoGUI_register_font(&MonoGUI_font_legible3x5_6ptFontInfo);
	
	MonoGUI_Sprite* bottomMenuOutlineSprite = MonoGUI_sprite_create(onDrawBottomMenuOutlineSprite);
	bottomMenuOutlineSpriteComponent = MonoGUI_component_create(MonoGUI_COMPONENT_SPRITE, 0, 56, bottomMenuOutlineSprite);
	
	fs2_prevButton			= MonoGUI_component_create(MonoGUI_COMPONENT_BUTTON,		MonoGFXDisplay.width / 4 + 2,		MonoGFXDisplay.height - LINE_HEIGHT, MonoGUI_button_create("   <", defaultFont, onPrevBtn));
	fs3_nextButton			= MonoGUI_component_create(MonoGUI_COMPONENT_BUTTON,		MonoGFXDisplay.width / 4 * 2 + 2,	MonoGFXDisplay.height - LINE_HEIGHT, MonoGUI_button_create("   >", defaultFont, onNextBtn));
	
	mainScreenSetup();
	scopeScreenSetup();
	functionGeneratorScreenSetup();
	bodeScreenSetup();
	
	UIIO_getButton(UIIO_BTN_1)->onClick = onSoftBtn1Click;
	UIIO_getButton(UIIO_BTN_2)->onClick = onSoftBtn2Click;
	UIIO_getButton(UIIO_BTN_3)->onClick = onSoftBtn3Click;
	UIIO_getButton(UIIO_BTN_4)->onClick = onSoftBtn4Click;
	UIIO_getButton(UIIO_BTN_MAIN)->onLongPress = onMainLongPress;
	UIIO_getButton(UIIO_BTN_MAIN)->onClick = onMainClick;
	UIIO_onScrollEvent = onScroll;
}

int32_t scrollCountr = 0;

// general event handlers
void onSoftBtn1Click(UIIO_BtnTypedef* btn) {
	if (MonoGUI_curr_screen == SCREEN_MAIN) {
		MonoGUI_click_component(ms1_scopeButton);
	} else if (MonoGUI_curr_screen == SCREEN_SCOPE) {
		MonoGUI_click_component(ss1_runStopToggleButtonComponent);
	} else if (MonoGUI_curr_screen == SCREEN_FNGEN) {
		MonoGUI_click_component(fs1_fnMenuButton);
	} else if (MonoGUI_curr_screen == SCREEN_BODE) {
		MonoGUI_click_component(bs1_measureButton);
	}
}
void onSoftBtn2Click(UIIO_BtnTypedef* btn) {
	if (MonoGUI_curr_screen == SCREEN_MAIN) {
		MonoGUI_click_component(ms2_fnGenButton);
	} else if (MonoGUI_curr_screen == SCREEN_SCOPE) {
		MonoGUI_click_component(fs2_prevButton);
	} else if (MonoGUI_curr_screen == SCREEN_FNGEN) {
		MonoGUI_click_component(fs2_prevButton);
	} else if (MonoGUI_curr_screen == SCREEN_BODE) {
		MonoGUI_click_component(bs2_fminScrollButtonComponent);
	}
}
void onSoftBtn3Click(UIIO_BtnTypedef* btn) {
	if (MonoGUI_curr_screen == SCREEN_MAIN) {
		MonoGUI_click_component(ms3_bodeButton);
	} else if (MonoGUI_curr_screen == SCREEN_SCOPE) {
		MonoGUI_click_component(fs3_nextButton);
	} else if (MonoGUI_curr_screen == SCREEN_FNGEN) {
		MonoGUI_click_component(fs3_nextButton);
	} else if (MonoGUI_curr_screen == SCREEN_BODE) {
		MonoGUI_click_component(bs3_fmaxScrollButtonComponent);
	}
}
void onSoftBtn4Click(UIIO_BtnTypedef* btn) {
	if (MonoGUI_curr_screen == SCREEN_MAIN) {
		MonoGUI_click_component(ms4_backlightScrollButton);
	} else if (MonoGUI_curr_screen == SCREEN_SCOPE) {
		MonoGUI_click_component(ss4_cfgMenuButtonComponent);
	} else if (MonoGUI_curr_screen == SCREEN_FNGEN) {
		MonoGUI_click_component(fs4_channelToggleButtonComponent);
	} else if (MonoGUI_curr_screen == SCREEN_BODE) {
		MonoGUI_click_component(bs4_toggleButton);
	}
}
void onMainLongPress(UIIO_BtnTypedef* mainBtn) {
	MonoGUI_set_screen(SCREEN_MAIN);
}
void onMainClick(UIIO_BtnTypedef* mainBtn) {
	MonoGUI_mainBtnClick();
}
void onScroll(int16_t delta) {
	MonoGUI_scroll(delta, (UIIO_getButton(UIIO_BTN_MAIN)->state & UIIO_BTN_STATE_PRESSED) ? 1 : 0);
	scrollCountr += delta;
}

// main screen
void onScopeBtnClick(void* caller) {
	MonoGUI_set_screen(SCREEN_SCOPE);
}
void onFnGenBtnClick(void* caller) {
	MonoGUI_set_screen(SCREEN_FNGEN);
}
void onBodeBtnClick(void* caller) {
	MonoGUI_set_screen(SCREEN_BODE);
}
void onBrightnessScroll(void* caller) {
	MonoGUI_ScrollButton* sb = (MonoGUI_ScrollButton*)caller;
	
	GLCD_setBacklight(sb->value / 100.0f);
}
void onDrawEncoderDebugSprite(MonoGUI_Sprite* sprite, int32_t x, int32_t y) {
	uint8_t currStateA = HAL_GPIO_ReadPin(ROT_ENC_A_GPIO_Port, ROT_ENC_A_Pin);
	uint8_t currStateB = HAL_GPIO_ReadPin(ROT_ENC_B_GPIO_Port, ROT_ENC_B_Pin);
	uint8_t lastStateA = !currStateA;
	uint8_t lastStateB = !currStateB;
	
	int8_t padding = 10;
	int8_t lineLen = 20;
	int8_t lineHeight = 10;
	
	MonoGFX_draw_line(x + padding, y + padding + (lastStateA ? 0 : lineHeight), x + padding + lineLen, y + padding + (lastStateA ? 0 : lineHeight), MonoGFX_COLOR_ON);
	MonoGFX_draw_line(x + padding + lineLen, y + padding + (lastStateA ? 0 : lineHeight), x + padding + lineLen, y + padding + (currStateA ? 0 : lineHeight), MonoGFX_COLOR_ON);
	MonoGFX_draw_line(x + padding + lineLen, y + padding + (currStateA ? 0 : lineHeight), x + padding + lineLen * 2, y + padding + (currStateA ? 0 : lineHeight), MonoGFX_COLOR_ON);
	MonoGUI_write_string(x + padding + lineLen * 2 + 5, y + padding, "A", 0, MonoGUI_TEXT_ALIGN_LEFT, MonoGUI_TEXT_DIRECTION_HORIZONTAL, MonoGFX_COLOR_ON);
	
	MonoGFX_draw_line(x + padding, y + lineHeight + padding * 2 + (lastStateB ? 0 : lineHeight), x + padding + lineLen, y + lineHeight + padding * 2 + (lastStateB ? 0 : lineHeight), MonoGFX_COLOR_ON);
	MonoGFX_draw_line(x + padding + lineLen, y + lineHeight + padding * 2 + (lastStateB ? 0 : lineHeight), x + padding + lineLen, y + lineHeight + padding * 2 + (currStateB ? 0 : lineHeight), MonoGFX_COLOR_ON);
	MonoGFX_draw_line(x + padding + lineLen, y + lineHeight + padding * 2 + (currStateB ? 0 : lineHeight), x + padding + lineLen * 2, y + lineHeight + padding * 2 + (currStateB ? 0 : lineHeight), MonoGFX_COLOR_ON);
	MonoGUI_write_string(x + padding + lineLen * 2 + 5, y + lineHeight + padding * 2, "B", 0, MonoGUI_TEXT_ALIGN_LEFT, MonoGUI_TEXT_DIRECTION_HORIZONTAL, MonoGFX_COLOR_ON);
	
	char tmpBuff[10];
	sprintf(tmpBuff, "%ld, %ld", TIM3->CNT, scrollCountr);
	MonoGUI_write_string(x + padding + lineLen * 2 + 10, y + padding + lineHeight, tmpBuff, 0, MonoGUI_TEXT_ALIGN_LEFT, MonoGUI_TEXT_DIRECTION_HORIZONTAL, MonoGFX_COLOR_ON);
}


// scope screen
void on1ACToggle(void* caller) {
	MonoGUI_ToggleButton* toggleBtn = (MonoGUI_ToggleButton*)caller;
	if (toggleBtn->checked) { // AC filter ON
		HAL_GPIO_WritePin(IN1_AC_GPIO_Port, IN1_AC_Pin, GPIO_PIN_RESET);
	} else { // AC filter OFF
		HAL_GPIO_WritePin(IN1_AC_GPIO_Port, IN1_AC_Pin, GPIO_PIN_SET);
	}
	
	ADC_setTriggerLevel(trigLevelLabel->scrollButton->value); // update trigger value for new offset
}
void on1AAToggle(void* caller) {
	MonoGUI_ToggleButton* toggleBtn = (MonoGUI_ToggleButton*)caller;
	if (toggleBtn->checked) {
		HAL_GPIO_WritePin(IN1_AA_GPIO_Port, IN1_AA_Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(IN1_AA_GPIO_Port, IN1_AA_Pin, GPIO_PIN_SET);
	}
}
void on2ACToggle(void* caller) {
	MonoGUI_ToggleButton* toggleBtn = (MonoGUI_ToggleButton*)caller;
	if (toggleBtn->checked) {
		HAL_GPIO_WritePin(IN2_AC_GPIO_Port, IN2_AC_Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(IN2_AC_GPIO_Port, IN2_AC_Pin, GPIO_PIN_SET);
	}
	
	ADC_setTriggerLevel(trigLevelLabel->scrollButton->value); // update trigger value for new offset
}
void on2AAToggle(void* caller) {
	MonoGUI_ToggleButton* toggleBtn = (MonoGUI_ToggleButton*)caller;
	if (toggleBtn->checked) {
		HAL_GPIO_WritePin(IN2_AA_GPIO_Port, IN2_AA_Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(IN2_AA_GPIO_Port, IN2_AA_Pin, GPIO_PIN_SET);
	}
}

void onModeSelect(void* caller) {
	MonoGUI_RadioButton* rbtn = MonoGUI_radioButton_get_selected(scopeModeNormalRadioButton);
	if (rbtn == scopeModeNormalRadioButton) {
		scopeGraph->mode = MonoGUI_GRAPH_MODE_12CH_NORMAL;
		hScrollLabelComponent->visible = 1;
	} else if (rbtn == scopeModeXYRadioButton) {
		scopeGraph->mode = MonoGUI_GRAPH_MODE_XY;
		hScrollLabelComponent->visible = 0;
	} else if (rbtn == scopeModeFFT1RadioButton) {
		scopeGraph->mode = MonoGUI_GRAPH_MODE_1CH_FFT;
		hScrollLabelComponent->visible = 1;
	} else if (rbtn == scopeModeFFT2RadioButton) {
		scopeGraph->mode = MonoGUI_GRAPH_MODE_2CH_FFT;
		hScrollLabelComponent->visible = 1;
	}
}
void updateScopeTriggerMode() {
	if (SCOPE_triggerEdge == SCOPE_TRIGGER_EDGE_RISING) {
		trigLevelLabel->scrollButton->button->label->value[2] = *MonoGUI_TRIG_RISING_CHAR;
	} else if (SCOPE_triggerEdge == SCOPE_TRIGGER_EDGE_FALLING) {
		trigLevelLabel->scrollButton->button->label->value[2] = *MonoGUI_TRIG_FALLING_CHAR;
	}
	if (SCOPE_triggerSource == SCOPE_TRIGGER_SOURCE_CH1) {
		trigLevelLabel->scrollButton->button->label->value[1] = 'A';
		trigLevelLabel->rightChannel = 0;
	} else if (SCOPE_triggerSource == SCOPE_TRIGGER_SOURCE_CH2) {
		trigLevelLabel->scrollButton->button->label->value[1] = 'B';
		trigLevelLabel->rightChannel = 1;
	}
	
	((MonoGUI_ToggleButton*)ss1_runStopToggleButtonComponent->component)->checked = 1;
	SCOPE_startConversion();
}
void onTriggerToggle(void* caller) {
	if (SCOPE_triggerSource == SCOPE_TRIGGER_SOURCE_CH1) {
		if (SCOPE_triggerEdge == SCOPE_TRIGGER_EDGE_RISING) { // A_RISING -> A_FALLING
			SCOPE_triggerEdge = SCOPE_TRIGGER_EDGE_FALLING;
		} else if (SCOPE_triggerEdge == SCOPE_TRIGGER_EDGE_FALLING) { // A_FALLING -> B_RISING
			SCOPE_triggerEdge = SCOPE_TRIGGER_EDGE_RISING;
			SCOPE_triggerSource = SCOPE_TRIGGER_SOURCE_CH2;
		}
	} else if (SCOPE_triggerSource == SCOPE_TRIGGER_SOURCE_CH2) {
		if (SCOPE_triggerEdge == SCOPE_TRIGGER_EDGE_RISING) { // B_RISING -> B_FALLING
			SCOPE_triggerEdge = SCOPE_TRIGGER_EDGE_FALLING;
		} else if (SCOPE_triggerEdge == SCOPE_TRIGGER_EDGE_FALLING) { // B_FALLING -> A_RISING
			SCOPE_triggerEdge = SCOPE_TRIGGER_EDGE_RISING;
			SCOPE_triggerSource = SCOPE_TRIGGER_SOURCE_CH1;
		}
	}
	
	updateScopeTriggerMode();
}
void onTriggerModeSelect(void* caller) {
	MonoGUI_RadioButton* rbtn = MonoGUI_radioButton_get_selected(trigModeNormalRadioButton);
	if (rbtn == trigModeNormalRadioButton) {
		if (SCOPE_triggerMode == SCOPE_TRIGGER_DISABLE) { // re-enable trigger
			SCOPE_triggerSource = SCOPE_TRIGGER_SOURCE_CH1;
			SCOPE_triggerEdge = SCOPE_TRIGGER_EDGE_RISING;
			updateScopeTriggerMode();
		}
		SCOPE_triggerMode = SCOPE_TRIGGER_CONTINUOUS;
		trigLevelLabelComponent->visible = 1;
	} else if (rbtn == trigModeSingleRadioButton) {
		if (SCOPE_triggerMode == SCOPE_TRIGGER_DISABLE) { // re-enable trigger
			SCOPE_triggerSource = SCOPE_TRIGGER_SOURCE_CH1;
			SCOPE_triggerEdge = SCOPE_TRIGGER_EDGE_RISING;
			updateScopeTriggerMode();
		}
		SCOPE_triggerMode = SCOPE_TRIGGER_SINGLE;
		trigLevelLabelComponent->visible = 1;
	} else if (rbtn == trigModeDisableRadioButton) {
		SCOPE_triggerMode = SCOPE_TRIGGER_DISABLE;
		SCOPE_triggerEdge = SCOPE_TRIGGER_EDGE_DISABLED;
		SCOPE_triggerSource = SCOPE_TRIGGER_SOURCE_DISABLED;
		trigLevelLabelComponent->visible = 0;
		
		updateScopeTriggerMode();
	}
}
void onRunStopToggle(void* caller) {
	if (((MonoGUI_ToggleButton*)ss1_runStopToggleButtonComponent->component)->checked) {
		SCOPE_startConversion();
	} else {
		SCOPE_setRunEnabled(0);
	}
}
void onScopeStop() {
	((MonoGUI_ToggleButton*)ss1_runStopToggleButtonComponent->component)->checked = 0;
}
void onTrigScroll(void* caller) {
	ADC_setTriggerLevel(trigLevelLabel->scrollButton->value);
}
void onHDivScroll(void* caller) {
	MonoGUI_ScrollButton* sb = (MonoGUI_ScrollButton*)caller;
	ADC_SetFrequencyID((uint32_t)sb->value);
	scopeGraph->Fs_Hz = ADC_GetFrequencySettings()->frequency;
}

// fngen screen
void drawFunctionSprite(MonoGUI_Sprite* sprite, int32_t x, int32_t y) {
	MonoGUI_RadioButton* rbtn = MonoGUI_radioButton_get_selected(fnSineRadioButton);
	
	uint32_t h = 29;
	uint32_t w = 55;
	float max = maxRange->scrollButton->value;
	float min = minRange->scrollButton->value;
	float duty = dutyRange->scrollButton->value;
	float rise = riseRange->scrollButton->value;
	float shape = shapeRange->scrollButton->value;
	if (rbtn == fnSineRadioButton) { // Sine
		for (uint32_t i = 0; i < w; i ++) {
			MonoGFX_set_pixel(x + i, y + h - ((sinf(i * 2.0f * M_PI / w) + 1.0f) * ((max - min) / 2) + min) * (h / maxRange->scrollButton->max), MonoGFX_COLOR_ON);
		}
	} else if (rbtn == fnSquareRadioButton) { // Square
		MonoGFX_draw_line(x, y + h - min * (h / maxRange->scrollButton->max), x + w * ((duty - rise / 2.0f) / 100.0f) - 1, y + h - min * (h / maxRange->scrollButton->max), MonoGFX_COLOR_ON);
		MonoGFX_draw_line(x + w * ((duty - rise / 2.0f) / 100.0f), y + h - min * (h / maxRange->scrollButton->max), x + w * ((duty + rise / 2.0f) / 100.0f), y + h - max * (h / maxRange->scrollButton->max), MonoGFX_COLOR_ON);
		MonoGFX_draw_line(x + w * ((duty + rise / 2.0f) / 100.0f), y + h - max * (h / maxRange->scrollButton->max), x + w - 1, y + h - max * (h / maxRange->scrollButton->max), MonoGFX_COLOR_ON);
	} else if (rbtn == fnTriRadioButton) { // Triangle
		MonoGFX_draw_line(x, y + h - min * (h / maxRange->scrollButton->max), x + w * (shape / 100.0f) - 1, y + h - max * (h / maxRange->scrollButton->max), MonoGFX_COLOR_ON);
		MonoGFX_draw_line(x + w * (shape / 100.0f) - 1, y + h - max * (h / maxRange->scrollButton->max), x + w - 1, y + h - min * (h / maxRange->scrollButton->max), MonoGFX_COLOR_ON);
	}
}

typedef struct {
	MonoGUI_RadioButton* waveformRadioButton;
	uint8_t bufferEnable;
	uint8_t outputEnable;
	float min;
	float max;
	float duty;
	float rise;
	float shape;
	float frequency;
} DACChannelConfigurationTypedef;

DACChannelConfigurationTypedef ch1Config = {.waveformRadioButton = NULL, .bufferEnable = 1, .outputEnable = 0, .min = 0.3f, .max = 3.0f, .duty = 50.0f, .rise = 0.0f, .shape = 50.0f, .frequency = 1000.0f};
DACChannelConfigurationTypedef ch2Config = {.waveformRadioButton = NULL, .bufferEnable = 1, .outputEnable = 0, .min = 0.3f, .max = 3.0f, .duty = 50.0f, .rise = 0.0f, .shape = 50.0f, .frequency = 1000.0f};

void setDACConfig(void* caller) {
	uint32_t ch;
	DACChannelConfigurationTypedef* chConfig;
	if (fs4_channelToggleButton->checked) { // channel 2
		ch = DAC_CHANNEL_2;
		chConfig = &ch2Config;
	} else { // channel 1
		ch = DAC_CHANNEL_1;
		chConfig = &ch1Config;
	}
	
	DAC_WaveformTypedef waveform = DAC_WAVEFORM_SINE;
	
	MonoGUI_RadioButton* rbtn = MonoGUI_radioButton_get_selected(fnSineRadioButton);
	
	if (rbtn == fnSineRadioButton) { // Sine
		waveform = DAC_WAVEFORM_SINE;
	} else if (rbtn == fnSquareRadioButton) { // Square
		waveform = DAC_WAVEFORM_SQUARE;
	} else if (rbtn == fnTriRadioButton) { // Triangle
		waveform = DAC_WAVEFORM_TRIANGLE;
	}
	float max = maxRange->scrollButton->value;
	float min = minRange->scrollButton->value;
	float duty = dutyRange->scrollButton->value / 100.0f;
	float rise = riseRange->scrollButton->value / 100.0f;
	float shape = shapeRange->scrollButton->value / 100.0f;
	
	chConfig->bufferEnable = outputBufferToggleButton->checked;
	chConfig->outputEnable = outputEnableToggleButton->checked;
	chConfig->waveformRadioButton = rbtn;
	chConfig->max = maxRange->scrollButton->value;
	chConfig->min = minRange->scrollButton->value;
	chConfig->duty = dutyRange->scrollButton->value;
	chConfig->rise = riseRange->scrollButton->value;
	chConfig->shape = shapeRange->scrollButton->value;
	chConfig->frequency = frequencyRange->scrollButton->value;
	
	if (outputEnableToggleButton->checked) {
		DAC_disable(ch);
		DAC_configure(ch, outputBufferToggleButton->checked, frequencyRange->scrollButton->value, waveform, min, max, (waveform == DAC_WAVEFORM_SQUARE) ? duty : shape, rise);
		DAC_enable(ch);
	} else {
		DAC_disable(ch);
	}
}
void onChannelSelect(void* caller) {
	DACChannelConfigurationTypedef* chConfig;
	if (fs4_channelToggleButton->checked) { // channel 2
		strcpy(fs_channelLabel->value, "CH 2");
		chConfig = &ch2Config;
	} else { // channel 1
		strcpy(fs_channelLabel->value, "CH 1");
		chConfig = &ch1Config;
	}
	outputBufferToggleButton->checked = chConfig->bufferEnable;
	outputEnableToggleButton->checked = chConfig->outputEnable;
	
	maxRange->scrollButton->value = chConfig->max;
	minRange->scrollButton->value = chConfig->min;
	dutyRange->scrollButton->value = chConfig->duty;
	riseRange->scrollButton->value = chConfig->rise;
	shapeRange->scrollButton->value = chConfig->shape;
	frequencyRange->scrollButton->value = chConfig->frequency;
	
	if (chConfig->waveformRadioButton) { // for non initialized channel 2
		MonoGUI_radioButton_click(chConfig->waveformRadioButton);
	} else {
		MonoGUI_radioButton_click(fnSineRadioButton); // DEFAULT CONFIG: select sine
	}
}
void onFunctionSelect(void* caller) {
	MonoGUI_RadioButton* rbtn = MonoGUI_radioButton_get_selected(fnSineRadioButton);
	
	if (rbtn == fnSineRadioButton) { // Sine
		MonoGUI_component_set_tabNext(frequencyRangeComponent, minRangeComponent);
		MonoGUI_component_set_tabNext(minRangeComponent, maxRangeComponent);
		MonoGUI_component_set_tabNext(maxRangeComponent, outputEnableToggleButtonComponent);
		MonoGUI_component_set_tabNext(outputEnableToggleButtonComponent, outputBufferToggleButtonComponent);
		MonoGUI_component_set_tabNext(outputBufferToggleButtonComponent, frequencyRangeComponent);
		
		dutyRangeComponent->visible = 0;
		riseRangeComponent->visible = 0;
		shapeRangeComponent->visible = 0;
	} else if (rbtn == fnSquareRadioButton) { // Square
		MonoGUI_component_set_tabNext(frequencyRangeComponent, minRangeComponent);
		MonoGUI_component_set_tabNext(minRangeComponent, maxRangeComponent);
		MonoGUI_component_set_tabNext(maxRangeComponent, dutyRangeComponent);
		MonoGUI_component_set_tabNext(dutyRangeComponent, riseRangeComponent);
		MonoGUI_component_set_tabNext(riseRangeComponent, outputEnableToggleButtonComponent);
		MonoGUI_component_set_tabNext(outputEnableToggleButtonComponent, outputBufferToggleButtonComponent);
		MonoGUI_component_set_tabNext(outputBufferToggleButtonComponent, frequencyRangeComponent);
		
		dutyRangeComponent->visible = 1;
		riseRangeComponent->visible = 1;
		shapeRangeComponent->visible = 0;
	} else if (rbtn == fnTriRadioButton) { // Triangle
		MonoGUI_component_set_tabNext(frequencyRangeComponent, minRangeComponent);
		MonoGUI_component_set_tabNext(minRangeComponent, maxRangeComponent);
		MonoGUI_component_set_tabNext(maxRangeComponent, shapeRangeComponent);
		MonoGUI_component_set_tabNext(shapeRangeComponent, outputEnableToggleButtonComponent);
		MonoGUI_component_set_tabNext(outputEnableToggleButtonComponent, outputBufferToggleButtonComponent);
		MonoGUI_component_set_tabNext(outputBufferToggleButtonComponent, frequencyRangeComponent);
		
		dutyRangeComponent->visible = 0;
		riseRangeComponent->visible = 0;
		shapeRangeComponent->visible = 1;
	}
	
	setDACConfig(caller);
}
void onFreqScroll(void* caller) {
	// TODO: implement native log scale support in range component
	if (frequencyRange->scrollButton->value < 10.0f) {
		frequencyRange->scrollButton->smallStep = 0.1f;
		frequencyRange->scrollButton->largeStep = 1.0f;
	} else if (frequencyRange->scrollButton->value < 100.0f) {
		frequencyRange->scrollButton->smallStep = 1.0f;
		frequencyRange->scrollButton->largeStep = 10.0f;
	} else if (frequencyRange->scrollButton->value < 1000.0f) {
		frequencyRange->scrollButton->smallStep = 10.0f;
		frequencyRange->scrollButton->largeStep = 100.0f;
	} else if (frequencyRange->scrollButton->value < 10000.0f) {
		frequencyRange->scrollButton->smallStep = 100.0f;
		frequencyRange->scrollButton->largeStep = 1000.0f;
	} else if (frequencyRange->scrollButton->value < 100000.0f) {
		frequencyRange->scrollButton->smallStep = 1000.0f;
		frequencyRange->scrollButton->largeStep = 10000.0f;
	} else if (frequencyRange->scrollButton->value < 1000000.0f) {
		frequencyRange->scrollButton->smallStep = 10000.0f;
		frequencyRange->scrollButton->largeStep = 100000.0f;
	}
	
	setDACConfig(caller);
}
void onMinScroll(void* caller) {
	if (minRange->scrollButton->value > maxRange->scrollButton->value) {
		minRange->scrollButton->value = maxRange->scrollButton->value;
	}
	
	setDACConfig(caller);
}
void onMaxScroll(void* caller) {
	if (maxRange->scrollButton->value < minRange->scrollButton->value) {
		maxRange->scrollButton->value = minRange->scrollButton->value;
	}
	
	setDACConfig(caller);
}
void onDutyScroll(void* caller) {
	if (dutyRange->scrollButton->value < riseRange->scrollButton->value / 2) {
		dutyRange->scrollButton->value = riseRange->scrollButton->value / 2;
	}
	if (dutyRange->scrollButton->value > riseRange->scrollButton->max - riseRange->scrollButton->value / 2) {
		dutyRange->scrollButton->value = riseRange->scrollButton->max - riseRange->scrollButton->value / 2;
	}
	
	setDACConfig(caller);
}
void onRiseScroll(void* caller) {
	if (riseRange->scrollButton->value > dutyRange->scrollButton->value * 2) {
		riseRange->scrollButton->value = dutyRange->scrollButton->value * 2;
	}
	if (riseRange->scrollButton->value > (dutyRange->scrollButton->max - dutyRange->scrollButton->value) * 2) {
		riseRange->scrollButton->value = (dutyRange->scrollButton->max - dutyRange->scrollButton->value) * 2;
	}
	
	setDACConfig(caller);
}
void onShapeScroll(void* caller) {
	setDACConfig(caller);
}
void onNextBtn(void* caller) {
	MonoGUI_close_all_menus();
	if (MonoGUI_selected_component == NULL) {
		MonoGUI_select_component(MonoGUI_screens[MonoGUI_curr_screen]->defaultTabComponent);
	} else {
		MonoGUI_tab(1);
	}
}
void onPrevBtn(void* caller) {
	MonoGUI_close_all_menus();
	if (MonoGUI_selected_component == NULL) {
		MonoGUI_select_component(MonoGUI_screens[MonoGUI_curr_screen]->defaultTabComponent);
		MonoGUI_tab(-1);
	} else {
		MonoGUI_tab(-1);
	}
}

// bode screen
void onBodeMeasureClick(void* caller) {
	BODE_startMeasurement((uint32_t)bs2_fminScrollButton->value);
}
void onDrawBodeProgressSprite(MonoGUI_Sprite* sprite, int32_t x, int32_t y) {
	float deltaX = 128 / BODE_NUM_CONFIGURATIONS;
	for (uint32_t i = 0; i < BODE_NUM_CONFIGURATIONS - 1; i ++) { 
		MonoGFX_draw_line((float)i * deltaX, 32.0f - (BODE_measurements[i].gain_dB / 2.0f), ((float)i + 1) * deltaX, 32.0f - (BODE_measurements[i + 1].gain_dB / 2.0f), MonoGFX_COLOR_ON);
		MonoGFX_draw_line_style((float)i * deltaX, 32.0f - (BODE_measurements[i].phase_deg / 4.0f), ((float)i + 1) * deltaX, 32.0f - (BODE_measurements[i + 1].phase_deg / 4.0f), MonoGFX_LINE_STYLE_DOTS, MonoGFX_COLOR_ON);
	}
	if (BODE_status != -1) {
		MonoGFX_fill_round_rect(x, y, 64, 7, 1, MonoGFX_COLOR_OFF); // clear background
		MonoGFX_draw_round_rect(x, y, 64, 7, 1, MonoGFX_COLOR_ON); // clear background
		
		float percent = ((float)(HAL_GetTick() - BODE_measurementStartTime)) / (float)BODE_measurementTime;
		MonoGFX_fill_rect(x + 2, y + 2, percent * 64.0f, 3, MonoGFX_COLOR_ON); // progress bar
	} else {
		
	}
}

void onDrawBottomMenuOutlineSprite(MonoGUI_Sprite* sprite, int32_t x, int32_t y) {
	MonoGFX_fill_rect(x, y, MonoGFXDisplay.width, 8, MonoGFX_COLOR_OFF); // clear background
	
	MonoGFX_draw_line(x, y, MonoGFXDisplay.width, y, MonoGFX_COLOR_ON);
	MonoGFX_draw_line(x + MonoGFXDisplay.width / 4, y, MonoGFXDisplay.width / 4, MonoGFXDisplay.height, MonoGFX_COLOR_ON);
	MonoGFX_draw_line(x + MonoGFXDisplay.width / 4 * 2, y, MonoGFXDisplay.width / 4 * 2, MonoGFXDisplay.height, MonoGFX_COLOR_ON);
	MonoGFX_draw_line(x + MonoGFXDisplay.width / 4 * 3, y, MonoGFXDisplay.width / 4 * 3, MonoGFXDisplay.height, MonoGFX_COLOR_ON);
}

