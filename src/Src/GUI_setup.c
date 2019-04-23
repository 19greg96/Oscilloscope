
#include "GUI_setup.h"
#include "GUI_font_legible3x5_6pt.h"
#include "BSP_UIIO.h"
#include "bode.h"

// TODO: rename components: [screen][Name][Type] example: ssTrigLevelLabel
// Main Screen
GUI_Component* ms1_scopeButton;
GUI_Component* ms2_fnGenButton;
GUI_Component* ms3_bodeButton;
GUI_Component* ms4_backlightScrollButton;
GUI_Component* bottomMenuOutlineSpriteComponent;

// Scope Screen
GUI_Graph* scopeGraph;

GUI_Component* ss1_runStopToggleButtonComponent;
GUI_Component* ss4_cfgMenuButtonComponent;

GUI_GraphLabel* trigLevelLabel;
GUI_Component* trigLevelLabelComponent;
GUI_GraphLabel* zeroLevelALabel;
GUI_GraphLabel* zeroLevelBLabel;
GUI_GraphLabel* hScrollLabel;
GUI_Component* hScrollLabelComponent;

GUI_RadioButton* trigModeNormalRadioButton;
GUI_RadioButton* trigModeSingleRadioButton;
GUI_RadioButton* trigModeDisableRadioButton;

GUI_RadioButton* scopeModeNormalRadioButton;
GUI_RadioButton* scopeModeXYRadioButton;
GUI_RadioButton* scopeModeFFT1RadioButton;
GUI_RadioButton* scopeModeFFT2RadioButton;

// Function generator Screen
GUI_Component* fs1_fnMenuButton;
GUI_Component* fs2_prevButton;
GUI_Component* fs3_nextButton;
GUI_ToggleButton* fs4_channelToggleButton;
GUI_Component* fs4_channelToggleButtonComponent;

GUI_Label* fs_channelLabel;
GUI_Range* frequencyRange;
GUI_Range* maxRange;
GUI_Range* minRange;
GUI_Range* riseRange;
GUI_Range* dutyRange;
GUI_Range* shapeRange;

GUI_Component* fs_channelLabelComponent;
GUI_Component* frequencyRangeComponent;
GUI_Component* maxRangeComponent;
GUI_Component* minRangeComponent;
GUI_Component* dutyRangeComponent;
GUI_Component* riseRangeComponent;
GUI_Component* shapeRangeComponent;

GUI_RadioButton* fnSineRadioButton;
GUI_RadioButton* fnSquareRadioButton;
GUI_RadioButton* fnTriRadioButton;

GUI_ToggleButton* outputEnableToggleButton;
GUI_ToggleButton* outputBufferToggleButton;
GUI_Component* outputEnableToggleButtonComponent;
GUI_Component* outputBufferToggleButtonComponent;

// Bode Screen
GUI_Component* bs1_measureButton;
GUI_Component* bs2_fminScrollButton;
GUI_Component* bs3_fmaxScrollButton;
GUI_Component* bs4_toggleButton;


// TODO: rename event handlers: on[Event][ComponentName][ComponentType] example: onClickSoftBtn1

// general event handlers
void onSoftBtn1Click(UIIO_BtnTypedef* btn);
void onSoftBtn2Click(UIIO_BtnTypedef* btn);
void onSoftBtn3Click(UIIO_BtnTypedef* btn);
void onSoftBtn4Click(UIIO_BtnTypedef* btn);
void onMainLongPress(UIIO_BtnTypedef* mainBtn);
void onMainClick(UIIO_BtnTypedef* mainBtn);
void onScroll(int32_t delta);

// main screen
void onScopeBtnClick(void* caller);
void onFnGenBtnClick(void* caller);
void onBodeBtnClick(void* caller);
void onBrightnessScroll(void* caller);

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
void drawFunctionSprite(GUI_Sprite* sprite, int32_t x, int32_t y);
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
void onDrawBodeProgressSprite(GUI_Sprite* sprite, int32_t x, int32_t y);

void onDrawBottomMenuOutlineSprite(GUI_Sprite* sprite, int32_t x, int32_t y);

uint32_t defaultFont;

void mainScreenSetup(){ // main screen
	GUI_Screen* functionSelectScreen = GUI_screen_create();
	GUI_screen_add_component(functionSelectScreen, bottomMenuOutlineSpriteComponent);
	
	GUI_ScrollButton* sb = GUI_scrollButton_create("Light", defaultFont, 0.0f, 100.0f, 50.0f, onBrightnessScroll);
	sb->smallStep = 1.0f;
	sb->largeStep = 5.0f;
	
	ms1_scopeButton			= GUI_component_create(GUI_COMPONENT_BUTTON,		2,							GLCD_height - LINE_HEIGHT, GUI_button_create("Scope", defaultFont, onScopeBtnClick));
	ms2_fnGenButton			= GUI_component_create(GUI_COMPONENT_BUTTON,		GLCD_width / 4 + 2,			GLCD_height - LINE_HEIGHT, GUI_button_create("Fn gen", defaultFont, onFnGenBtnClick));
	ms3_bodeButton			= GUI_component_create(GUI_COMPONENT_BUTTON,		GLCD_width / 4 * 2 + 2,		GLCD_height - LINE_HEIGHT, GUI_button_create("Bode", defaultFont, onBodeBtnClick));
	ms4_backlightScrollButton= GUI_component_create(GUI_COMPONENT_SCROLL_BUTTON,GLCD_width / 4 * 3 + 2,		GLCD_height - LINE_HEIGHT, sb);
	
	GUI_scrollButton_scroll(sb, 0, 0); // DEFAULT CONFIG
	
	GUI_screen_add_component(functionSelectScreen, ms1_scopeButton);
	GUI_screen_add_component(functionSelectScreen, ms2_fnGenButton);
	GUI_screen_add_component(functionSelectScreen, ms3_bodeButton);
	GUI_screen_add_component(functionSelectScreen, ms4_backlightScrollButton);
}

void scopeScreenSetup(float* outputBuffer1_V, float* outputBuffer2_V) { // scope screen
	GUI_Screen* scopeScreen = GUI_screen_create();
	
	scopeGraph = GUI_graph_create(GLCD_width, GLCD_height - 9);
	scopeGraph->data_1 = outputBuffer1_V;
	scopeGraph->data_2 = outputBuffer2_V;
	scopeGraph->dataBufferSize = ADC_INPUT_BUFFER_SIZE / 2;
	scopeGraph->mode = GUI_GRAPH_MODE_12CH_NORMAL;
	arm_rfft_fast_init_f32(&scopeGraph->FFTStruct, scopeGraph->dataBufferSize); // this graph will be able to run FFT (TODO: this should be done in graph or in global math handler)
	
	GUI_Component* scopeGraphComponent = GUI_component_create(GUI_COMPONENT_GRAPH, 0, 0, scopeGraph);
	
	zeroLevelALabel = GUI_graphLabel_create("A0", GLCD_LINE_STYLE_SPARSE_DOTS, 0, 1, defaultFont, NULL);
	GUI_Component* zeroLevelALabelComponent = GUI_component_create(GUI_COMPONENT_GRAPH_LABEL, 0, 0, zeroLevelALabel);
	zeroLevelALabel->scrollButton->min = -10.0f;
	zeroLevelALabel->scrollButton->max = 10.0f;
	
	zeroLevelALabel->nextLabel = zeroLevelALabelComponent;
	scopeGraph->vOffset1GraphLabel = zeroLevelALabel;
	GUI_graph_add_label(scopeGraph, zeroLevelALabelComponent);
	
	zeroLevelBLabel = GUI_graphLabel_create("B0", GLCD_LINE_STYLE_SPARSE_DOTS, 0, 1, defaultFont, NULL); // TODO: B channel zero level should only be visible when B channel is enabled
	zeroLevelBLabel->rightChannel = 1;
	GUI_Component* zeroLevelBLabelComponent = GUI_component_create(GUI_COMPONENT_GRAPH_LABEL, 0, 0, zeroLevelBLabel);
	zeroLevelBLabel->nextLabel = zeroLevelBLabelComponent;
	scopeGraph->vOffset2GraphLabel = zeroLevelBLabel;
	GUI_graph_add_label(scopeGraph, zeroLevelBLabelComponent);
	
	trigLevelLabel = GUI_graphLabel_create("TA~", GLCD_LINE_STYLE_SPARSE_DOTS, 0, 1, defaultFont, onTrigScroll);
	trigLevelLabelComponent = GUI_component_create(GUI_COMPONENT_GRAPH_LABEL, 0, 0, trigLevelLabel);
	trigLevelLabel->scrollButton->button->onClick = onTriggerToggle;
	trigLevelLabel->nextLabel = trigLevelLabelComponent;
	GUI_graph_add_label(scopeGraph, trigLevelLabelComponent);
	GUI_scrollButton_scroll(trigLevelLabel->scrollButton, 0, 0); // DEFAULT CONFIG: set trigger level
	
	hScrollLabel = GUI_graphLabel_create("", GLCD_LINE_STYLE_SPARSE_DOTS, 0, 0, defaultFont, NULL);
	hScrollLabelComponent = GUI_component_create(GUI_COMPONENT_GRAPH_LABEL, 0, 0, hScrollLabel);
	hScrollLabel->nextLabel = hScrollLabelComponent;
	scopeGraph->hOffsetGraphLabel = hScrollLabel;
	GUI_graph_add_label(scopeGraph, hScrollLabelComponent);
	
	scopeScreen->defaultTabComponent = scopeGraph->hDivScrollButtonComponent;
	GUI_component_set_tabNext(scopeGraph->hDivScrollButtonComponent, scopeGraph->vDiv1ScrollButtonComponent);
	GUI_component_set_tabNext(scopeGraph->vDiv1ScrollButtonComponent, scopeGraph->vDiv2ScrollButtonComponent);
	GUI_component_set_tabNext(scopeGraph->vDiv2ScrollButtonComponent, trigLevelLabelComponent);
	GUI_component_set_tabNext(trigLevelLabelComponent, zeroLevelALabelComponent);
	GUI_component_set_tabNext(zeroLevelALabelComponent, zeroLevelBLabelComponent);
	GUI_component_set_tabNext(zeroLevelBLabelComponent, hScrollLabelComponent);
	GUI_component_set_tabNext(hScrollLabelComponent, scopeGraph->hDivScrollButtonComponent);
	
	GUI_screen_add_component(scopeScreen, scopeGraphComponent);
	
	GUI_screen_add_component(scopeScreen, bottomMenuOutlineSpriteComponent);
	
	zeroLevelALabel->scrollButton->min = 0.0f;
	zeroLevelALabel->scrollButton->max = 5.0f;
	zeroLevelALabel->scrollButton->value = 0.0f;
	zeroLevelALabel->scrollButton->largeStep = 0.1f;
	zeroLevelALabel->scrollButton->smallStep = 0.01f;
	
	zeroLevelBLabel->scrollButton->min = 0.0f;
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
	
	((GUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->min = 0.0f;
	((GUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->max = (float)(numADCFrequencySettings - 1);
	((GUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->value = 0.0f;
	((GUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->largeStep = 2.0f;
	((GUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->smallStep = 1.0f;
	((GUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->onScroll = onHDivScroll;
	((GUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component)->labelIsValue = 0;
	GUI_scrollButton_scroll(((GUI_ScrollButton*)scopeGraph->hDivScrollButtonComponent->component), 0, 0); // DEFAULT CONFIG: set horizontal div
	
	((GUI_ScrollButton*)scopeGraph->vDiv1ScrollButtonComponent->component)->min = 0.01f;
	((GUI_ScrollButton*)scopeGraph->vDiv1ScrollButtonComponent->component)->max = 2.0f;
	((GUI_ScrollButton*)scopeGraph->vDiv1ScrollButtonComponent->component)->value = 1.0f;
	((GUI_ScrollButton*)scopeGraph->vDiv1ScrollButtonComponent->component)->largeStep = 0.1f;
	((GUI_ScrollButton*)scopeGraph->vDiv1ScrollButtonComponent->component)->smallStep = 0.01f;
	
	((GUI_ScrollButton*)scopeGraph->vDiv2ScrollButtonComponent->component)->min = 0.01f;
	((GUI_ScrollButton*)scopeGraph->vDiv2ScrollButtonComponent->component)->max = 2.0f;
	((GUI_ScrollButton*)scopeGraph->vDiv2ScrollButtonComponent->component)->value = 1.0f;
	((GUI_ScrollButton*)scopeGraph->vDiv2ScrollButtonComponent->component)->largeStep = 0.1f;
	((GUI_ScrollButton*)scopeGraph->vDiv2ScrollButtonComponent->component)->smallStep = 0.01f;
	
	GUI_ToggleButton* ss1_runStopToggleButton = GUI_toggleButton_create("   RUN", defaultFont, onRunStopToggle);
	ss1_runStopToggleButton->showCheckbox = 0;
	strcpy(ss1_runStopToggleButton->checkedText, "  STOP");
	ss1_runStopToggleButtonComponent = GUI_component_create(GUI_COMPONENT_TOGGLE_BUTTON, 2, GLCD_height - LINE_HEIGHT, ss1_runStopToggleButton);
	GUI_toggleButton_click(ss1_runStopToggleButton); // DEFAULT CONFIG: run
	
	
	GUI_Menu* cfgMenu = GUI_menu_create();
	
	GUI_MenuColumn* trigSourceMenuColumn = GUI_menuColumn_create();
	trigModeNormalRadioButton = GUI_radioButton_create("T Norm", defaultFont, onTriggerModeSelect);
	trigModeSingleRadioButton = GUI_radioButton_create("T Once", defaultFont, onTriggerModeSelect);
	trigModeDisableRadioButton = GUI_radioButton_create("T Off", defaultFont, onTriggerModeSelect);
	GUI_Component* trigModeNormalRadioButtonComponent = GUI_component_create(GUI_COMPONENT_RADIO_BUTTON, 0, 0, trigModeNormalRadioButton);
	GUI_Component* trigModeSingleRadioButtonComponent = GUI_component_create(GUI_COMPONENT_RADIO_BUTTON, 0, 0, trigModeSingleRadioButton);
	GUI_Component* trigModeDisableRadioButtonComponent = GUI_component_create(GUI_COMPONENT_RADIO_BUTTON, 0, 0, trigModeDisableRadioButton);
	GUI_menuColumn_add_component(trigSourceMenuColumn, trigModeNormalRadioButtonComponent);
	GUI_menuColumn_add_component(trigSourceMenuColumn, trigModeSingleRadioButtonComponent);
	GUI_menuColumn_add_component(trigSourceMenuColumn, trigModeDisableRadioButtonComponent);
	
	GUI_radioButton_click(trigModeDisableRadioButton); // DEFAULT CONFIG: select trigger off mode
	
	GUI_MenuColumn* scopeModeMenuColumn = GUI_menuColumn_create();
	scopeModeNormalRadioButton = GUI_radioButton_create("Normal", defaultFont, onModeSelect);
	scopeModeXYRadioButton = GUI_radioButton_create("XY", defaultFont, onModeSelect);
	scopeModeFFT1RadioButton = GUI_radioButton_create("FFT 1", defaultFont, onModeSelect);
	scopeModeFFT2RadioButton = GUI_radioButton_create("FFT 2", defaultFont, onModeSelect);
	GUI_Component* scopeModeNormalRadioButtonComponent = GUI_component_create(GUI_COMPONENT_RADIO_BUTTON, 0, 0, scopeModeNormalRadioButton);
	GUI_Component* scopeModeXYRadioButtonComponent = GUI_component_create(GUI_COMPONENT_RADIO_BUTTON, 0, 0, scopeModeXYRadioButton);
	GUI_Component* scopeModeFFT1RadioButtonComponent = GUI_component_create(GUI_COMPONENT_RADIO_BUTTON, 0, 0, scopeModeFFT1RadioButton);
	GUI_Component* scopeModeFFT2RadioButtonComponent = GUI_component_create(GUI_COMPONENT_RADIO_BUTTON, 0, 0, scopeModeFFT2RadioButton);
	GUI_menuColumn_add_component(scopeModeMenuColumn, scopeModeNormalRadioButtonComponent);
	GUI_menuColumn_add_component(scopeModeMenuColumn, scopeModeXYRadioButtonComponent);
	GUI_menuColumn_add_component(scopeModeMenuColumn, scopeModeFFT1RadioButtonComponent);
	GUI_menuColumn_add_component(scopeModeMenuColumn, scopeModeFFT2RadioButtonComponent);
	
	GUI_radioButton_click(scopeModeNormalRadioButton); // DEFAULT CONFIG: select normal scope mode
	
	GUI_MenuColumn* inputFilterMenuColumn = GUI_menuColumn_create();
	GUI_ToggleButton* inputFilter1ACToggleButton = GUI_toggleButton_create("1AC", defaultFont, on1ACToggle);
	GUI_ToggleButton* inputFilter1AAToggleButton = GUI_toggleButton_create("1AA", defaultFont, on1AAToggle);
	GUI_ToggleButton* inputFilter2ACToggleButton = GUI_toggleButton_create("2AC", defaultFont, on2ACToggle);
	GUI_ToggleButton* inputFilter2AAToggleButton = GUI_toggleButton_create("2AA", defaultFont, on2AAToggle);
	
	GUI_toggleButton_click(inputFilter1ACToggleButton); // DEFAULT CONFIG: relay configuration
	GUI_toggleButton_click(inputFilter1AAToggleButton); // DEFAULT CONFIG: relay configuration
	GUI_toggleButton_click(inputFilter2ACToggleButton); // DEFAULT CONFIG: relay configuration
	GUI_toggleButton_click(inputFilter2AAToggleButton); // DEFAULT CONFIG: relay configuration
	
	GUI_Component* inputFilter1ACToggleButtonComponent = GUI_component_create(GUI_COMPONENT_TOGGLE_BUTTON, 0, 0, inputFilter1ACToggleButton);
	GUI_Component* inputFilter1AAToggleButtonComponent = GUI_component_create(GUI_COMPONENT_TOGGLE_BUTTON, 0, 0, inputFilter1AAToggleButton);
	GUI_Component* inputFilter2ACToggleButtonComponent = GUI_component_create(GUI_COMPONENT_TOGGLE_BUTTON, 0, 0, inputFilter2ACToggleButton);
	GUI_Component* inputFilter2AAToggleButtonComponent = GUI_component_create(GUI_COMPONENT_TOGGLE_BUTTON, 0, 0, inputFilter2AAToggleButton);
	GUI_menuColumn_add_component(inputFilterMenuColumn, inputFilter1ACToggleButtonComponent);
	GUI_menuColumn_add_component(inputFilterMenuColumn, inputFilter1AAToggleButtonComponent);
	GUI_menuColumn_add_component(inputFilterMenuColumn, inputFilter2ACToggleButtonComponent);
	GUI_menuColumn_add_component(inputFilterMenuColumn, inputFilter2AAToggleButtonComponent);
	
	GUI_menu_add_column(cfgMenu, trigSourceMenuColumn);
	GUI_menu_add_column(cfgMenu, scopeModeMenuColumn);
	GUI_menu_add_column(cfgMenu, inputFilterMenuColumn);
	ss4_cfgMenuButtonComponent		= GUI_component_create(GUI_COMPONENT_MENU_BUTTON,	GLCD_width / 4 * 3 + 2,		GLCD_height - LINE_HEIGHT, GUI_menuButton_create("Cfg", defaultFont, NULL, cfgMenu));
	GUI_screen_add_component(scopeScreen, ss1_runStopToggleButtonComponent);
	// GUI_screen_add_component(scopeScreen, ss2_vdivScrollButtonComponent);
	GUI_screen_add_component(scopeScreen, fs2_prevButton);
	GUI_screen_add_component(scopeScreen, fs3_nextButton);
	// GUI_screen_add_component(scopeScreen, ss3_trigScrollButtonComponent);
	GUI_screen_add_component(scopeScreen, ss4_cfgMenuButtonComponent);
	
	SCOPE_handleOnStop = onScopeStop;
}

void functionGeneratorScreenSetup() { // Fn gen screen
	GUI_Screen* fnGenScreen = GUI_screen_create();
	
	GUI_screen_add_component(fnGenScreen, bottomMenuOutlineSpriteComponent);
	
	GUI_Sprite* fnSprite = GUI_sprite_create(drawFunctionSprite);
	GUI_Component* fnSpriteComponent = GUI_component_create(GUI_COMPONENT_SPRITE, 44, 1, fnSprite);
	GUI_screen_add_component(fnGenScreen, fnSpriteComponent);
	// Setting buttons
	fs_channelLabel = GUI_label_create("CH 1", defaultFont);
	fs_channelLabelComponent = GUI_component_create(GUI_COMPONENT_LABEL, 102, 32, fs_channelLabel);
	GUI_screen_add_component(fnGenScreen, fs_channelLabelComponent);
	outputEnableToggleButton = GUI_toggleButton_create("Output", defaultFont, setDACConfig);
	outputBufferToggleButton = GUI_toggleButton_create("Buffer", defaultFont, setDACConfig);
	outputEnableToggleButtonComponent = GUI_component_create(GUI_COMPONENT_TOGGLE_BUTTON, 102, 40, outputEnableToggleButton);
	outputBufferToggleButtonComponent = GUI_component_create(GUI_COMPONENT_TOGGLE_BUTTON, 102, 48, outputBufferToggleButton);
	GUI_screen_add_component(fnGenScreen, outputEnableToggleButtonComponent);
	GUI_screen_add_component(fnGenScreen, outputBufferToggleButtonComponent);
	
	frequencyRange = GUI_range_create("1 Freq", defaultFont, 1.0f, 1000000.1f, 1000.0f, onFreqScroll, 38, formatFrequency);
	frequencyRangeComponent = GUI_component_create(GUI_COMPONENT_RANGE, 1, 38, frequencyRange);
	GUI_screen_add_component(fnGenScreen, frequencyRangeComponent);
	fnGenScreen->defaultTabComponent = frequencyRangeComponent;
	// Sine
	minRange = GUI_range_create("2 Min", defaultFont, 0.0f, 3.3f, 0.3f, onMinScroll, 29, formatVoltage);
	minRange->vertical = 1;
	minRange->scrollButton->button->label->align = GUI_TEXT_ALIGN_RIGHT;
	minRange->scrollButton->smallStep = 0.01f;
	minRange->scrollButton->largeStep = 0.1f;
	minRange->valueLabel->align = GUI_TEXT_ALIGN_RIGHT;
	minRangeComponent = GUI_component_create(GUI_COMPONENT_RANGE, 41, 1, minRange);
	GUI_screen_add_component(fnGenScreen, minRangeComponent);
	
	maxRange = GUI_range_create("3 Max", defaultFont, 0.0f, 3.3f, 3.0f, onMaxScroll, 29, formatVoltage);
	maxRange->vertical = 1;
	maxRange->scrollButton->smallStep = 0.01f;
	maxRange->scrollButton->largeStep = 0.1f;
	maxRangeComponent = GUI_component_create(GUI_COMPONENT_RANGE, 101, 1, maxRange);
	GUI_screen_add_component(fnGenScreen, maxRangeComponent);
	// Square
	dutyRange = GUI_range_create("4 Duty", defaultFont, 0.0f, 100.0f, 50.0f, onDutyScroll, 54, formatPercentage);
	dutyRange->scrollButton->smallStep = 1.0f;
	dutyRange->scrollButton->largeStep = 10.0f;
	dutyRange->valueLabel->align = GUI_TEXT_ALIGN_RIGHT;
	dutyRangeComponent = GUI_component_create(GUI_COMPONENT_RANGE, 44, 33, dutyRange);
	GUI_screen_add_component(fnGenScreen, dutyRangeComponent);
	
	riseRange = GUI_range_create("5 Rise", defaultFont, 0.0f, 100.0f, 0.0f, onRiseScroll, 54, formatPercentage);
	riseRange->scrollButton->smallStep = 1.0f;
	riseRange->scrollButton->largeStep = 10.0f;
	riseRange->valueLabel->align = GUI_TEXT_ALIGN_RIGHT;
	riseRangeComponent = GUI_component_create(GUI_COMPONENT_RANGE, 44, 45, riseRange);
	GUI_screen_add_component(fnGenScreen, riseRangeComponent);
	// Triangle
	shapeRange = GUI_range_create("4 Shape", defaultFont, 0.0f, 100.0f, 50.0f, onShapeScroll, 54, formatPercentage);
	shapeRange->scrollButton->smallStep = 1.0f;
	shapeRange->scrollButton->largeStep = 10.0f;
	shapeRange->valueLabel->align = GUI_TEXT_ALIGN_RIGHT;
	shapeRangeComponent = GUI_component_create(GUI_COMPONENT_RANGE, 44, 33, shapeRange);
	GUI_screen_add_component(fnGenScreen, shapeRangeComponent);
	
	GUI_Menu* fnMenu = GUI_menu_create();
	
	GUI_MenuColumn* fnMenuColumn = GUI_menuColumn_create();
	fnSineRadioButton = GUI_radioButton_create("Sine", defaultFont, onFunctionSelect);
	GUI_Component* fnSineRadioButtonComponent = GUI_component_create(GUI_COMPONENT_RADIO_BUTTON, 0, 0, fnSineRadioButton);
	fnSquareRadioButton = GUI_radioButton_create("Square", defaultFont, onFunctionSelect);
	GUI_Component* fnSquareRadioButtonComponent = GUI_component_create(GUI_COMPONENT_RADIO_BUTTON, 0, 0, fnSquareRadioButton);
	fnTriRadioButton = GUI_radioButton_create("Tri", defaultFont, onFunctionSelect);
	GUI_Component* fnTriRadioButtonComponent = GUI_component_create(GUI_COMPONENT_RADIO_BUTTON, 0, 0, fnTriRadioButton);
	GUI_menuColumn_add_component(fnMenuColumn, fnSineRadioButtonComponent);
	GUI_menuColumn_add_component(fnMenuColumn, fnSquareRadioButtonComponent);
	GUI_menuColumn_add_component(fnMenuColumn, fnTriRadioButtonComponent);
	
	GUI_menu_add_column(fnMenu, fnMenuColumn);
	GUI_radioButton_click(fnSineRadioButton); // DEFAULT CONFIG: select sine
	
	fs1_fnMenuButton		= GUI_component_create(GUI_COMPONENT_MENU_BUTTON,	2,						GLCD_height - LINE_HEIGHT, GUI_menuButton_create("Fn", defaultFont, NULL, fnMenu));
	
	fs4_channelToggleButton = GUI_toggleButton_create("Toggle", defaultFont, onChannelSelect);
	fs4_channelToggleButton->showCheckbox = 0;
	fs4_channelToggleButtonComponent = GUI_component_create(GUI_COMPONENT_TOGGLE_BUTTON, GLCD_width / 4 * 3 + 2,	GLCD_height - LINE_HEIGHT, fs4_channelToggleButton);
	GUI_screen_add_component(fnGenScreen, fs1_fnMenuButton);
	GUI_screen_add_component(fnGenScreen, fs2_prevButton);
	GUI_screen_add_component(fnGenScreen, fs3_nextButton);
	GUI_screen_add_component(fnGenScreen, fs4_channelToggleButtonComponent);
}

void bodeScreenSetup() { // Bode screen
	GUI_Screen* bodeScreen = GUI_screen_create();
	GUI_screen_add_component(bodeScreen, bottomMenuOutlineSpriteComponent);
	
	bs1_measureButton		= GUI_component_create(GUI_COMPONENT_BUTTON,		2,						GLCD_height - LINE_HEIGHT, GUI_button_create("Measure", defaultFont, onBodeMeasureClick));
	bs2_fminScrollButton	= GUI_component_create(GUI_COMPONENT_SCROLL_BUTTON,	GLCD_width / 4 + 2,		GLCD_height - LINE_HEIGHT, GUI_scrollButton_create("Fmin", defaultFont, 0, 10, 1, NULL));
	bs3_fmaxScrollButton	= GUI_component_create(GUI_COMPONENT_SCROLL_BUTTON,	GLCD_width / 4 * 2 + 2,	GLCD_height - LINE_HEIGHT, GUI_scrollButton_create("Fmax", defaultFont, 0, 10, 1, NULL));
	bs4_toggleButton		= GUI_component_create(GUI_COMPONENT_BUTTON,		GLCD_width / 4 * 3 + 2,	GLCD_height - LINE_HEIGHT, GUI_button_create("Toggle", defaultFont, NULL));
	
	GUI_Sprite* bsProgressSprite = GUI_sprite_create(onDrawBodeProgressSprite);
	GUI_Component* bsProgressSpriteComponent = GUI_component_create(GUI_COMPONENT_SPRITE, 32, 28, bsProgressSprite);
	GUI_screen_add_component(bodeScreen, bsProgressSpriteComponent);
	
	GUI_screen_add_component(bodeScreen, bs1_measureButton);
	GUI_screen_add_component(bodeScreen, bs2_fminScrollButton);
	GUI_screen_add_component(bodeScreen, bs3_fmaxScrollButton);
	GUI_screen_add_component(bodeScreen, bs4_toggleButton);
	
	// TODO: bode graph
}

void GUI_setup_init(float* outputBuffer1_V, float* outputBuffer2_V) {
	defaultFont = GUI_register_font(&GUI_font_legible3x5_6ptFontInfo);
	
	GUI_Sprite* bottomMenuOutlineSprite = GUI_sprite_create(onDrawBottomMenuOutlineSprite);
	bottomMenuOutlineSpriteComponent = GUI_component_create(GUI_COMPONENT_SPRITE, 0, 56, bottomMenuOutlineSprite);
	
	fs2_prevButton			= GUI_component_create(GUI_COMPONENT_BUTTON,		GLCD_width / 4 + 2,		GLCD_height - LINE_HEIGHT, GUI_button_create("   <", defaultFont, onPrevBtn));
	fs3_nextButton			= GUI_component_create(GUI_COMPONENT_BUTTON,		GLCD_width / 4 * 2 + 2,	GLCD_height - LINE_HEIGHT, GUI_button_create("   >", defaultFont, onNextBtn));
	
	mainScreenSetup();
	scopeScreenSetup(outputBuffer1_V, outputBuffer2_V);
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


// general event handlers
void onSoftBtn1Click(UIIO_BtnTypedef* btn) {
	if (GUI_curr_screen == SCREEN_MAIN) {
		GUI_click_component(ms1_scopeButton);
	} else if (GUI_curr_screen == SCREEN_SCOPE) {
		GUI_click_component(ss1_runStopToggleButtonComponent);
	} else if (GUI_curr_screen == SCREEN_FNGEN) {
		GUI_click_component(fs1_fnMenuButton);
	} else if (GUI_curr_screen == SCREEN_BODE) {
		GUI_click_component(bs1_measureButton);
	}
}
void onSoftBtn2Click(UIIO_BtnTypedef* btn) {
	if (GUI_curr_screen == SCREEN_MAIN) {
		GUI_click_component(ms2_fnGenButton);
	} else if (GUI_curr_screen == SCREEN_SCOPE) {
		GUI_click_component(fs2_prevButton);
	} else if (GUI_curr_screen == SCREEN_FNGEN) {
		GUI_click_component(fs2_prevButton);
	} else if (GUI_curr_screen == SCREEN_BODE) {
		GUI_click_component(bs2_fminScrollButton);
	}
}
void onSoftBtn3Click(UIIO_BtnTypedef* btn) {
	if (GUI_curr_screen == SCREEN_MAIN) {
		GUI_click_component(ms3_bodeButton);
	} else if (GUI_curr_screen == SCREEN_SCOPE) {
		GUI_click_component(fs3_nextButton);
	} else if (GUI_curr_screen == SCREEN_FNGEN) {
		GUI_click_component(fs3_nextButton);
	} else if (GUI_curr_screen == SCREEN_BODE) {
		GUI_click_component(bs3_fmaxScrollButton);
	}
}
void onSoftBtn4Click(UIIO_BtnTypedef* btn) {
	if (GUI_curr_screen == SCREEN_MAIN) {
		GUI_click_component(ms4_backlightScrollButton);
	} else if (GUI_curr_screen == SCREEN_SCOPE) {
		GUI_click_component(ss4_cfgMenuButtonComponent);
	} else if (GUI_curr_screen == SCREEN_FNGEN) {
		GUI_click_component(fs4_channelToggleButtonComponent);
	} else if (GUI_curr_screen == SCREEN_BODE) {
		GUI_click_component(bs4_toggleButton);
	}
}
void onMainLongPress(UIIO_BtnTypedef* mainBtn) {
	GUI_set_screen(SCREEN_MAIN);
}
void onMainClick(UIIO_BtnTypedef* mainBtn) {
	GUI_mainBtnClick();
}
void onScroll(int32_t delta) {
	GUI_scroll(delta, (UIIO_getButton(UIIO_BTN_MAIN)->state & UIIO_BTN_STATE_PRESSED) ? 1 : 0);
}

// main screen
void onScopeBtnClick(void* caller) {
	GUI_set_screen(SCREEN_SCOPE);
}
void onFnGenBtnClick(void* caller) {
	GUI_set_screen(SCREEN_FNGEN);
}
void onBodeBtnClick(void* caller) {
	GUI_set_screen(SCREEN_BODE);
}
void onBrightnessScroll(void* caller) {
	GUI_ScrollButton* sb = (GUI_ScrollButton*)caller;
	
	GLCD_setBacklight(sb->value / 100.0f);
}

// scope screen
void on1ACToggle(void* caller) {
	GUI_ToggleButton* toggleBtn = (GUI_ToggleButton*)caller;
	if (toggleBtn->checked) { // AC filter ON
		HAL_GPIO_WritePin(IN1_AC_GPIO_Port, IN1_AC_Pin, GPIO_PIN_RESET);
	} else { // AC filter OFF
		HAL_GPIO_WritePin(IN1_AC_GPIO_Port, IN1_AC_Pin, GPIO_PIN_SET);
	}
	
	ADC_setTriggerLevel(trigLevelLabel->scrollButton->value); // update trigger value for new offset
}
void on1AAToggle(void* caller) {
	GUI_ToggleButton* toggleBtn = (GUI_ToggleButton*)caller;
	if (toggleBtn->checked) {
		HAL_GPIO_WritePin(IN1_AA_GPIO_Port, IN1_AA_Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(IN1_AA_GPIO_Port, IN1_AA_Pin, GPIO_PIN_SET);
	}
}
void on2ACToggle(void* caller) {
	GUI_ToggleButton* toggleBtn = (GUI_ToggleButton*)caller;
	if (toggleBtn->checked) {
		HAL_GPIO_WritePin(IN2_AC_GPIO_Port, IN2_AC_Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(IN2_AC_GPIO_Port, IN2_AC_Pin, GPIO_PIN_SET);
	}
	
	ADC_setTriggerLevel(trigLevelLabel->scrollButton->value); // update trigger value for new offset
}
void on2AAToggle(void* caller) {
	GUI_ToggleButton* toggleBtn = (GUI_ToggleButton*)caller;
	if (toggleBtn->checked) {
		HAL_GPIO_WritePin(IN2_AA_GPIO_Port, IN2_AA_Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(IN2_AA_GPIO_Port, IN2_AA_Pin, GPIO_PIN_SET);
	}
}

void onModeSelect(void* caller) {
	GUI_RadioButton* rbtn = GUI_radioButton_get_selected(scopeModeNormalRadioButton);
	if (rbtn == scopeModeNormalRadioButton) {
		scopeGraph->mode = GUI_GRAPH_MODE_12CH_NORMAL;
		hScrollLabelComponent->visible = 1;
	} else if (rbtn == scopeModeXYRadioButton) {
		scopeGraph->mode = GUI_GRAPH_MODE_XY;
		hScrollLabelComponent->visible = 0;
	} else if (rbtn == scopeModeFFT1RadioButton) {
		scopeGraph->mode = GUI_GRAPH_MODE_1CH_FFT;
		hScrollLabelComponent->visible = 1;
	} else if (rbtn == scopeModeFFT2RadioButton) {
		scopeGraph->mode = GUI_GRAPH_MODE_2CH_FFT;
		hScrollLabelComponent->visible = 1;
	}
}
void updateScopeTriggerMode() {
	if (SCOPE_triggerEdge == SCOPE_TRIGGER_EDGE_RISING) {
		trigLevelLabel->scrollButton->button->label->value[2] = *GUI_TRIG_RISING_CHAR;
	} else if (SCOPE_triggerEdge == SCOPE_TRIGGER_EDGE_FALLING) {
		trigLevelLabel->scrollButton->button->label->value[2] = *GUI_TRIG_FALLING_CHAR;
	}
	if (SCOPE_triggerSource == SCOPE_TRIGGER_SOURCE_CH1) {
		trigLevelLabel->scrollButton->button->label->value[1] = 'A';
		trigLevelLabel->rightChannel = 0;
	} else if (SCOPE_triggerSource == SCOPE_TRIGGER_SOURCE_CH2) {
		trigLevelLabel->scrollButton->button->label->value[1] = 'B';
		trigLevelLabel->rightChannel = 1;
	}
	
	((GUI_ToggleButton*)ss1_runStopToggleButtonComponent->component)->checked = 1;
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
	GUI_RadioButton* rbtn = GUI_radioButton_get_selected(trigModeNormalRadioButton);
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
	if (((GUI_ToggleButton*)ss1_runStopToggleButtonComponent->component)->checked) {
		SCOPE_startConversion();
	} else {
		SCOPE_setRunEnabled(0);
	}
}
void onScopeStop() {
	((GUI_ToggleButton*)ss1_runStopToggleButtonComponent->component)->checked = 0;
}
void onTrigScroll(void* caller) {
	ADC_setTriggerLevel(trigLevelLabel->scrollButton->value);
}
void onHDivScroll(void* caller) {
	GUI_ScrollButton* sb = (GUI_ScrollButton*)caller;
	ADC_SetFrequencyID((uint32_t)sb->value);
	scopeGraph->Fs_Hz = ADC_GetFrequencySettings()->frequency;
}

// fngen screen
void drawFunctionSprite(GUI_Sprite* sprite, int32_t x, int32_t y) {
	GUI_RadioButton* rbtn = GUI_radioButton_get_selected(fnSineRadioButton);
	
	uint32_t h = 29;
	uint32_t w = 55;
	float max = maxRange->scrollButton->value;
	float min = minRange->scrollButton->value;
	float duty = dutyRange->scrollButton->value;
	float rise = riseRange->scrollButton->value;
	float shape = shapeRange->scrollButton->value;
	if (rbtn == fnSineRadioButton) { // Sine
		for (uint32_t i = 0; i < w; i ++) {
			GLCD_set_pixel(x + i, y + h - ((sinf(i * 2.0f * M_PI / w) + 1.0f) * ((max - min) / 2) + min) * (h / maxRange->scrollButton->max), GLCD_COLOR_ON);
		}
	} else if (rbtn == fnSquareRadioButton) { // Square
		GLCD_draw_line(x, y + h - min * (h / maxRange->scrollButton->max), x + w * ((duty - rise / 2.0f) / 100.0f) - 1, y + h - min * (h / maxRange->scrollButton->max), GLCD_COLOR_ON);
		GLCD_draw_line(x + w * ((duty - rise / 2.0f) / 100.0f), y + h - min * (h / maxRange->scrollButton->max), x + w * ((duty + rise / 2.0f) / 100.0f), y + h - max * (h / maxRange->scrollButton->max), GLCD_COLOR_ON);
		GLCD_draw_line(x + w * ((duty + rise / 2.0f) / 100.0f), y + h - max * (h / maxRange->scrollButton->max), x + w - 1, y + h - max * (h / maxRange->scrollButton->max), GLCD_COLOR_ON);
	} else if (rbtn == fnTriRadioButton) { // Triangle
		GLCD_draw_line(x, y + h - min * (h / maxRange->scrollButton->max), x + w * (shape / 100.0f) - 1, y + h - max * (h / maxRange->scrollButton->max), GLCD_COLOR_ON);
		GLCD_draw_line(x + w * (shape / 100.0f) - 1, y + h - max * (h / maxRange->scrollButton->max), x + w - 1, y + h - min * (h / maxRange->scrollButton->max), GLCD_COLOR_ON);
	}
}

typedef struct {
	GUI_RadioButton* waveform;
	uint8_t bufferEnable;
	uint8_t outputEnable;
	float min;
	float max;
	float duty;
	float rise;
	float shape;
	float frequency;
} DACChannelConfigurationTypedef;

DACChannelConfigurationTypedef ch1Config = {0};
DACChannelConfigurationTypedef ch2Config = {0};

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
	
	GUI_RadioButton* rbtn = GUI_radioButton_get_selected(fnSineRadioButton);
	
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
	chConfig->waveform = rbtn;
	chConfig->max = maxRange->scrollButton->value;
	chConfig->min = minRange->scrollButton->value;
	chConfig->duty = dutyRange->scrollButton->value;
	chConfig->rise = riseRange->scrollButton->value;
	chConfig->shape = shapeRange->scrollButton->value;
	chConfig->frequency = frequencyRange->scrollButton->value;
	
	if (outputEnableToggleButton->checked) {
		DAC_configure(ch, frequencyRange->scrollButton->value, waveform, min, max, (waveform == DAC_WAVEFORM_SQUARE) ? duty : shape, rise);
		DAC_enable(ch);
	} else {
		DAC_disable(ch);
	}
	/*
	TODO: 1. DAC output buffer toggle buttons
	GUI_ToggleButton* output1BuffToggleButton;
	GUI_ToggleButton* output2BuffToggleButton;
	*/
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
	
	if (chConfig->waveform) { // for non initialized channel 2
		GUI_radioButton_click(chConfig->waveform);
	} else {
		GUI_radioButton_click(fnSineRadioButton); // DEFAULT CONFIG: select sine
	}
}
void onFunctionSelect(void* caller) {
	GUI_RadioButton* rbtn = GUI_radioButton_get_selected(fnSineRadioButton);
	
	if (rbtn == fnSineRadioButton) { // Sine
		GUI_component_set_tabNext(frequencyRangeComponent, minRangeComponent);
		GUI_component_set_tabNext(minRangeComponent, maxRangeComponent);
		GUI_component_set_tabNext(maxRangeComponent, outputEnableToggleButtonComponent);
		GUI_component_set_tabNext(outputEnableToggleButtonComponent, outputBufferToggleButtonComponent);
		GUI_component_set_tabNext(outputBufferToggleButtonComponent, frequencyRangeComponent);
		
		dutyRangeComponent->visible = 0;
		riseRangeComponent->visible = 0;
		shapeRangeComponent->visible = 0;
	} else if (rbtn == fnSquareRadioButton) { // Square
		GUI_component_set_tabNext(frequencyRangeComponent, minRangeComponent);
		GUI_component_set_tabNext(minRangeComponent, maxRangeComponent);
		GUI_component_set_tabNext(maxRangeComponent, dutyRangeComponent);
		GUI_component_set_tabNext(dutyRangeComponent, riseRangeComponent);
		GUI_component_set_tabNext(riseRangeComponent, outputEnableToggleButtonComponent);
		GUI_component_set_tabNext(outputEnableToggleButtonComponent, outputBufferToggleButtonComponent);
		GUI_component_set_tabNext(outputBufferToggleButtonComponent, frequencyRangeComponent);
		
		dutyRangeComponent->visible = 1;
		riseRangeComponent->visible = 1;
		shapeRangeComponent->visible = 0;
	} else if (rbtn == fnTriRadioButton) { // Triangle
		GUI_component_set_tabNext(frequencyRangeComponent, minRangeComponent);
		GUI_component_set_tabNext(minRangeComponent, maxRangeComponent);
		GUI_component_set_tabNext(maxRangeComponent, shapeRangeComponent);
		GUI_component_set_tabNext(shapeRangeComponent, outputEnableToggleButtonComponent);
		GUI_component_set_tabNext(outputEnableToggleButtonComponent, outputBufferToggleButtonComponent);
		GUI_component_set_tabNext(outputBufferToggleButtonComponent, frequencyRangeComponent);
		
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
	GUI_close_all_menus();
	if (GUI_selected_component == NULL) {
		GUI_select_component(GUI_screens[GUI_curr_screen]->defaultTabComponent);
	} else {
		GUI_tab(1);
	}
}
void onPrevBtn(void* caller) {
	GUI_close_all_menus();
	if (GUI_selected_component == NULL) {
		GUI_select_component(GUI_screens[GUI_curr_screen]->defaultTabComponent);
	} else {
		GUI_tab(-1);
	}
}

// bode screen
void onBodeMeasureClick(void* caller) {
	BODE_startMeasurement();
}
void onDrawBodeProgressSprite(GUI_Sprite* sprite, int32_t x, int32_t y) {
	float deltaX = 128 / BODE_NUM_CONFIGURATIONS;
	for (uint32_t i = 0; i < BODE_NUM_CONFIGURATIONS - 1; i ++) { 
		GLCD_draw_line((float)i * deltaX, 32.0f - (BODE_measurements[i].gain_dB / 2.0f), ((float)i + 1) * deltaX, 32.0f - (BODE_measurements[i + 1].gain_dB / 2.0f), GLCD_COLOR_ON);
		GLCD_draw_line_style((float)i * deltaX, 32.0f - (BODE_measurements[i].phase_deg / 4.0f), ((float)i + 1) * deltaX, 32.0f - (BODE_measurements[i + 1].phase_deg / 4.0f), GLCD_LINE_STYLE_DOTS, GLCD_COLOR_ON);
	}
	if (BODE_status != -1) {
		GLCD_fill_round_rect(x, y, 64, 7, 1, GLCD_COLOR_OFF); // clear background
		GLCD_draw_round_rect(x, y, 64, 7, 1, GLCD_COLOR_ON); // clear background
		
		float percent = ((float)(HAL_GetTick() - BODE_measurementStartTime)) / (float)BODE_measurementTime;
		GLCD_fill_rect(x + 2, y + 2, percent * 64.0f, 3, GLCD_COLOR_ON); // progress bar
	} else {
		
	}
}

void onDrawBottomMenuOutlineSprite(GUI_Sprite* sprite, int32_t x, int32_t y) {
	GLCD_fill_rect(x, y, GLCD_width, 8, GLCD_COLOR_OFF); // clear background
	
	GLCD_draw_line(x, y, GLCD_width, y, GLCD_COLOR_ON);
	GLCD_draw_line(x + GLCD_width / 4, y, GLCD_width / 4, GLCD_height, GLCD_COLOR_ON);
	GLCD_draw_line(x + GLCD_width / 4 * 2, y, GLCD_width / 4 * 2, GLCD_height, GLCD_COLOR_ON);
	GLCD_draw_line(x + GLCD_width / 4 * 3, y, GLCD_width / 4 * 3, GLCD_height, GLCD_COLOR_ON);
}

