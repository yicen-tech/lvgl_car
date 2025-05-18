# Copyright 2023 NXP
# SPDX-License-Identifier: MIT
# The auto-generated can only be used on NXP devices

import SDL
import utime as time
import usys as sys
import lvgl as lv
import lodepng as png
import ustruct

lv.init()
SDL.init(w=1024,h=600)

# Register SDL display driver.
disp_buf1 = lv.disp_draw_buf_t()
buf1_1 = bytearray(1024*10)
disp_buf1.init(buf1_1, None, len(buf1_1)//4)
disp_drv = lv.disp_drv_t()
disp_drv.init()
disp_drv.draw_buf = disp_buf1
disp_drv.flush_cb = SDL.monitor_flush
disp_drv.hor_res = 1024
disp_drv.ver_res = 600
disp_drv.register()

# Regsiter SDL mouse driver
indev_drv = lv.indev_drv_t()
indev_drv.init() 
indev_drv.type = lv.INDEV_TYPE.POINTER
indev_drv.read_cb = SDL.mouse_read
indev_drv.register()

# Below: Taken from https://github.com/lvgl/lv_binding_micropython/blob/master/driver/js/imagetools.py#L22-L94

COLOR_SIZE = lv.color_t.__SIZE__
COLOR_IS_SWAPPED = hasattr(lv.color_t().ch,'green_h')

class lodepng_error(RuntimeError):
    def __init__(self, err):
        if type(err) is int:
            super().__init__(png.error_text(err))
        else:
            super().__init__(err)

# Parse PNG file header
# Taken from https://github.com/shibukawa/imagesize_py/blob/ffef30c1a4715c5acf90e8945ceb77f4a2ed2d45/imagesize.py#L63-L85

def get_png_info(decoder, src, header):
    # Only handle variable image types

    if lv.img.src_get_type(src) != lv.img.SRC.VARIABLE:
        return lv.RES.INV

    data = lv.img_dsc_t.__cast__(src).data
    if data == None:
        return lv.RES.INV

    png_header = bytes(data.__dereference__(24))

    if png_header.startswith(b'\211PNG\r\n\032\n'):
        if png_header[12:16] == b'IHDR':
            start = 16
        # Maybe this is for an older PNG version.
        else:
            start = 8
        try:
            width, height = ustruct.unpack(">LL", png_header[start:start+8])
        except ustruct.error:
            return lv.RES.INV
    else:
        return lv.RES.INV

    header.always_zero = 0
    header.w = width
    header.h = height
    header.cf = lv.img.CF.TRUE_COLOR_ALPHA

    return lv.RES.OK

def convert_rgba8888_to_bgra8888(img_view):
    for i in range(0, len(img_view), lv.color_t.__SIZE__):
        ch = lv.color_t.__cast__(img_view[i:i]).ch
        ch.red, ch.blue = ch.blue, ch.red

# Read and parse PNG file

def open_png(decoder, dsc):
    img_dsc = lv.img_dsc_t.__cast__(dsc.src)
    png_data = img_dsc.data
    png_size = img_dsc.data_size
    png_decoded = png.C_Pointer()
    png_width = png.C_Pointer()
    png_height = png.C_Pointer()
    error = png.decode32(png_decoded, png_width, png_height, png_data, png_size)
    if error:
        raise lodepng_error(error)
    img_size = png_width.int_val * png_height.int_val * 4
    img_data = png_decoded.ptr_val
    img_view = img_data.__dereference__(img_size)

    if COLOR_SIZE == 4:
        convert_rgba8888_to_bgra8888(img_view)
    else:
        raise lodepng_error("Error: Color mode not supported yet!")

    dsc.img_data = img_data
    return lv.RES.OK

# Above: Taken from https://github.com/lvgl/lv_binding_micropython/blob/master/driver/js/imagetools.py#L22-L94

decoder = lv.img.decoder_create()
decoder.info_cb = get_png_info
decoder.open_cb = open_png

def anim_x_cb(obj, v):
    obj.set_x(v)

def anim_y_cb(obj, v):
    obj.set_y(v)

def ta_event_cb(e,kb):
    code = e.get_code()
    ta = e.get_target()
    if code == lv.EVENT.FOCUSED:
        kb.set_textarea(ta)
        kb.move_foreground()
        kb.clear_flag(lv.obj.FLAG.HIDDEN)

    if code == lv.EVENT.DEFOCUSED:
        kb.set_textarea(None)
        kb.move_background()
        kb.add_flag(lv.obj.FLAG.HIDDEN)
        
def ta_zh_event_cb(e,kb):
    code = e.get_code()
    ta = e.get_target()
    if code == lv.EVENT.FOCUSED:
        kb.keyboard_set_textarea(ta)
        kb.move_foreground()
        kb.clear_flag(lv.obj.FLAG.HIDDEN)

    if code == lv.EVENT.DEFOCUSED:
        kb.keyboard_set_textarea(None)
        kb.move_background()
        kb.add_flag(lv.obj.FLAG.HIDDEN)



# create home
home = lv.obj()
home.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_home_main_main_default
style_home_main_main_default = lv.style_t()
style_home_main_main_default.init()
style_home_main_main_default.set_bg_color(lv.color_make(0x00,0x5c,0xff))
style_home_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_home_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_home_main_main_default.set_bg_opa(255)

# add style for home
home.add_style(style_home_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create home_music_btn
home_music_btn = lv.imgbtn(home)
home_music_btn.set_pos(int(310),int(519))
home_music_btn.set_size(70,70)
home_music_btn.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
try:
    with open('D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp-1117518030.png','rb') as f:
        home_music_btn_imgReleased_data = f.read()
except:
    print('Could not open D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp-1117518030.png')
    sys.exit()

home_music_btn_imgReleased = lv.img_dsc_t({
  'data_size': len(home_music_btn_imgReleased_data),
  'header': {'always_zero': 0, 'w': 70, 'h': 70, 'cf': lv.img.CF.TRUE_COLOR_ALPHA},
  'data': home_music_btn_imgReleased_data
})
home_music_btn.set_src(lv.imgbtn.STATE.RELEASED, None, home_music_btn_imgReleased, None)





home_music_btn.add_flag(lv.obj.FLAG.CHECKABLE)
# create style style_home_music_btn_main_main_default
style_home_music_btn_main_main_default = lv.style_t()
style_home_music_btn_main_main_default.init()
style_home_music_btn_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_home_music_btn_main_main_default.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_home_music_btn_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_home_music_btn_main_main_default.set_text_font(lv.font_montserrat_16)
style_home_music_btn_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_home_music_btn_main_main_default.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_home_music_btn_main_main_default.set_img_recolor_opa(0)
style_home_music_btn_main_main_default.set_img_opa(255)

# add style for home_music_btn
home_music_btn.add_style(style_home_music_btn_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_home_music_btn_main_main_pressed
style_home_music_btn_main_main_pressed = lv.style_t()
style_home_music_btn_main_main_pressed.init()
style_home_music_btn_main_main_pressed.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_home_music_btn_main_main_pressed.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_home_music_btn_main_main_pressed.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_home_music_btn_main_main_pressed.set_text_font(lv.font_montserrat_16)
style_home_music_btn_main_main_pressed.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_home_music_btn_main_main_pressed.set_img_recolor_opa(0)
style_home_music_btn_main_main_pressed.set_img_opa(255)

# add style for home_music_btn
home_music_btn.add_style(style_home_music_btn_main_main_pressed, lv.PART.MAIN|lv.STATE.PRESSED)

# create style style_home_music_btn_main_main_checked
style_home_music_btn_main_main_checked = lv.style_t()
style_home_music_btn_main_main_checked.init()
style_home_music_btn_main_main_checked.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_home_music_btn_main_main_checked.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_home_music_btn_main_main_checked.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_home_music_btn_main_main_checked.set_text_font(lv.font_montserrat_16)
style_home_music_btn_main_main_checked.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_home_music_btn_main_main_checked.set_img_recolor_opa(0)
style_home_music_btn_main_main_checked.set_img_opa(255)

# add style for home_music_btn
home_music_btn.add_style(style_home_music_btn_main_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)


# create home_system_name
home_system_name = lv.spangroup(home)
home_system_name.set_pos(int(429),int(31))
home_system_name.set_size(166,26)
home_system_name.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
home_system_name.set_align(lv.TEXT_ALIGN.LEFT)
home_system_name.set_overflow(lv.SPAN_OVERFLOW.CLIP)
home_system_name.set_mode(lv.SPAN_MODE.BREAK)
home_system_name_span = home_system_name.new_span()
home_system_name_span.set_text("车载终端系统")
home_system_name_span.style.set_text_color(lv.color_make(0xff,0xff,0xff))
home_system_name_span.style.set_text_decor(lv.TEXT_DECOR.NONE)
try:
    home_system_name_span.style.set_text_font(lv.font_simsun_26)
except AttributeError:
    try:
        home_system_name_span.style.set_text_font(lv.font_montserrat_26)
    except AttributeError:
        home_system_name_span.style.set_text_font(lv.font_montserrat_16)
home_system_name.refr_mode()
# create style style_home_system_name_main_main_default
style_home_system_name_main_main_default = lv.style_t()
style_home_system_name_main_main_default.init()
style_home_system_name_main_main_default.set_radius(0)
style_home_system_name_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_home_system_name_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_home_system_name_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_home_system_name_main_main_default.set_bg_opa(0)
style_home_system_name_main_main_default.set_border_color(lv.color_make(0x00,0x00,0x00))
style_home_system_name_main_main_default.set_border_width(0)
style_home_system_name_main_main_default.set_border_opa(255)
style_home_system_name_main_main_default.set_pad_left(0)
style_home_system_name_main_main_default.set_pad_right(0)
style_home_system_name_main_main_default.set_pad_top(0)
style_home_system_name_main_main_default.set_pad_bottom(0)

# add style for home_system_name
home_system_name.add_style(style_home_system_name_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create home_setting_btn
home_setting_btn = lv.imgbtn(home)
home_setting_btn.set_pos(int(656),int(520))
home_setting_btn.set_size(70,70)
home_setting_btn.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
try:
    with open('D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp2004044731.png','rb') as f:
        home_setting_btn_imgReleased_data = f.read()
except:
    print('Could not open D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp2004044731.png')
    sys.exit()

home_setting_btn_imgReleased = lv.img_dsc_t({
  'data_size': len(home_setting_btn_imgReleased_data),
  'header': {'always_zero': 0, 'w': 70, 'h': 70, 'cf': lv.img.CF.TRUE_COLOR_ALPHA},
  'data': home_setting_btn_imgReleased_data
})
home_setting_btn.set_src(lv.imgbtn.STATE.RELEASED, None, home_setting_btn_imgReleased, None)





home_setting_btn.add_flag(lv.obj.FLAG.CHECKABLE)
# create style style_home_setting_btn_main_main_default
style_home_setting_btn_main_main_default = lv.style_t()
style_home_setting_btn_main_main_default.init()
style_home_setting_btn_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_home_setting_btn_main_main_default.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_home_setting_btn_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_home_setting_btn_main_main_default.set_text_font(lv.font_montserrat_16)
style_home_setting_btn_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_home_setting_btn_main_main_default.set_img_recolor(lv.color_make(0xff,0xff,0xff))
style_home_setting_btn_main_main_default.set_img_recolor_opa(0)
style_home_setting_btn_main_main_default.set_img_opa(255)

# add style for home_setting_btn
home_setting_btn.add_style(style_home_setting_btn_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_home_setting_btn_main_main_pressed
style_home_setting_btn_main_main_pressed = lv.style_t()
style_home_setting_btn_main_main_pressed.init()
style_home_setting_btn_main_main_pressed.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_home_setting_btn_main_main_pressed.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_home_setting_btn_main_main_pressed.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_home_setting_btn_main_main_pressed.set_text_font(lv.font_montserrat_16)
style_home_setting_btn_main_main_pressed.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_home_setting_btn_main_main_pressed.set_img_recolor_opa(0)
style_home_setting_btn_main_main_pressed.set_img_opa(255)

# add style for home_setting_btn
home_setting_btn.add_style(style_home_setting_btn_main_main_pressed, lv.PART.MAIN|lv.STATE.PRESSED)

# create style style_home_setting_btn_main_main_checked
style_home_setting_btn_main_main_checked = lv.style_t()
style_home_setting_btn_main_main_checked.init()
style_home_setting_btn_main_main_checked.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_home_setting_btn_main_main_checked.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_home_setting_btn_main_main_checked.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_home_setting_btn_main_main_checked.set_text_font(lv.font_montserrat_16)
style_home_setting_btn_main_main_checked.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_home_setting_btn_main_main_checked.set_img_recolor_opa(0)
style_home_setting_btn_main_main_checked.set_img_opa(255)

# add style for home_setting_btn
home_setting_btn.add_style(style_home_setting_btn_main_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)


# create home_digital_clock_1

home_digital_clock_1 = lv.dclock(home, "11:25:50")
home_digital_clock_1.set_text("11:25:50")



class home_digital_clock_1_timerClass():
    def __init__(self):
        self.hour = 11
        self.minute = 25
        self.second = 50
  
    def count_24(self, timer):
        self.second += 1
        if self.second == 60:
            self.second = 0
            self.minute += 1
	
        if self.minute == 60:
            self.minute = 0
            self.hour +=1
            
        if self.hour == 24:
            self.hour = 0

        home_digital_clock_1.set_text("%02d:%02d:%02d" %(self.hour, self.minute, self.second))

    def count_12(self, timer):
        self.second += 1
        if self.second == 60:
            self.second = 0
            self.minute += 1
        if self.minute == 60:
            self.minute = 0
            if self.hour < 12:
                self.hour += 1
            else:
                self.hour += 1
                self.hour = self.hour % 12
        if (self.hour == 12 and self.second == 0 and self.minute == 0):
            if(self.meridiem == "PM"):
                self.meridiem = "AM"
            else:
                self.meridiem = "PM"
		
        home_digital_clock_1.set_text("%02d:%02d:%02d %s" %(self.hour, self.minute, self.second, self.meridiem))

home_digital_clock_1_timerInstance = home_digital_clock_1_timerClass()

home_digital_clock_1_timer = lv.timer_create_basic()
home_digital_clock_1_timer.set_period(1000)

home_digital_clock_1_timer.set_cb(lambda src: home_digital_clock_1_timerInstance.count_24(home_digital_clock_1_timer))
lv.dclock.set_style_text_align(home_digital_clock_1, lv.TEXT_ALIGN.CENTER, 0);

home_digital_clock_1.set_pos(int(17),int(66))
home_digital_clock_1.set_size(132,47)
home_digital_clock_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_home_digital_clock_1_main_main_default
style_home_digital_clock_1_main_main_default = lv.style_t()
style_home_digital_clock_1_main_main_default.init()
style_home_digital_clock_1_main_main_default.set_radius(0)
style_home_digital_clock_1_main_main_default.set_bg_color(lv.color_make(0x00,0x00,0x00))
style_home_digital_clock_1_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_home_digital_clock_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_home_digital_clock_1_main_main_default.set_bg_opa(0)
style_home_digital_clock_1_main_main_default.set_text_color(lv.color_make(0xff,0xff,0xff))
try:
    style_home_digital_clock_1_main_main_default.set_text_font(lv.font_montserratMedium_29)
except AttributeError:
    try:
        style_home_digital_clock_1_main_main_default.set_text_font(lv.font_montserrat_29)
    except AttributeError:
        style_home_digital_clock_1_main_main_default.set_text_font(lv.font_montserrat_16)
style_home_digital_clock_1_main_main_default.set_text_letter_space(0)
style_home_digital_clock_1_main_main_default.set_pad_left(0)
style_home_digital_clock_1_main_main_default.set_pad_right(0)
style_home_digital_clock_1_main_main_default.set_pad_top(7)
style_home_digital_clock_1_main_main_default.set_pad_bottom(0)

# add style for home_digital_clock_1
home_digital_clock_1.add_style(style_home_digital_clock_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create home_datetext_1
home_datetext_1 = lv.label(home)
home_datetext_1.set_text("2022/07/28")
home_datetext_1.set_style_text_align(lv.TEXT_ALIGN.CENTER, 0)

try:
    home_datetext_1_calendar
except NameError:
    home_datetext_1_calendar = lv.calendar(lv.layer_top())
    home_datetext_1_calendar.set_size(240, 240)
    home_datetext_1_calendar.set_pos(1,0)
    home_datetext_1_calendar.set_showed_date(2022, 5)
    home_datetext_1_calendar.align(lv.ALIGN.CENTER, 20, 20)
    lv.calendar_header_arrow(home_datetext_1_calendar)

home_datetext_1_calendar.add_flag(lv.obj.FLAG.HIDDEN)

def home_datetext_1_calendar_event_handler(e, textV):
    code = e.get_code()
    obj = e.get_current_target()
    if code == lv.EVENT.VALUE_CHANGED:
        date = lv.calendar_date_t()
        obj.get_pressed_date(date) == lv.RES.OK
        textV.set_text("%02d/%02d/%02d"%(date.year, date.month, date.day))

def home_datetext_1_event(e, date_ca):
    code = e.get_code()
    if code == lv.EVENT.FOCUSED:
        date_ca.add_event_cb(lambda e: home_datetext_1_calendar_event_handler(e, home_datetext_1), lv.EVENT.ALL, None)
        date_ca.move_foreground()
        date_ca.clear_flag(lv.obj.FLAG.HIDDEN)

    if code == lv.EVENT.DEFOCUSED:
        date_ca.move_background()
        date_ca.add_flag(lv.obj.FLAG.HIDDEN)

home_datetext_1.add_flag(lv.obj.FLAG.CLICKABLE)
home_datetext_1.add_event_cb(lambda e: home_datetext_1_event(e, home_datetext_1_calendar), lv.EVENT.ALL, None)
home_datetext_1.set_pos(int(19),int(12))
home_datetext_1.set_size(130,36)
home_datetext_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_home_datetext_1_main_main_default
style_home_datetext_1_main_main_default = lv.style_t()
style_home_datetext_1_main_main_default.init()
style_home_datetext_1_main_main_default.set_radius(0)
style_home_datetext_1_main_main_default.set_bg_color(lv.color_make(0xfb,0xb6,0xb6))
style_home_datetext_1_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_home_datetext_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_home_datetext_1_main_main_default.set_bg_opa(0)
style_home_datetext_1_main_main_default.set_text_color(lv.color_make(0xff,0xff,0xff))
try:
    style_home_datetext_1_main_main_default.set_text_font(lv.font_montserratMedium_16)
except AttributeError:
    try:
        style_home_datetext_1_main_main_default.set_text_font(lv.font_montserrat_16)
    except AttributeError:
        style_home_datetext_1_main_main_default.set_text_font(lv.font_montserrat_16)
style_home_datetext_1_main_main_default.set_text_letter_space(2)
style_home_datetext_1_main_main_default.set_pad_left(0)
style_home_datetext_1_main_main_default.set_pad_right(0)
style_home_datetext_1_main_main_default.set_pad_top(7)

# add style for home_datetext_1
home_datetext_1.add_style(style_home_datetext_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create home_school_img
home_school_img = lv.img(home)
home_school_img.set_pos(int(874),int(0))
home_school_img.set_size(150,150)
home_school_img.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
home_school_img.add_flag(lv.obj.FLAG.CLICKABLE)
try:
    with open('D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp-1680125814.png','rb') as f:
        home_school_img_img_data = f.read()
except:
    print('Could not open D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp-1680125814.png')
    sys.exit()

home_school_img_img = lv.img_dsc_t({
  'data_size': len(home_school_img_img_data),
  'header': {'always_zero': 0, 'w': 150, 'h': 150, 'cf': lv.img.CF.TRUE_COLOR_ALPHA},
  'data': home_school_img_img_data
})

home_school_img.set_src(home_school_img_img)
home_school_img.set_pivot(50,50)
home_school_img.set_angle(0)
# create style style_home_school_img_main_main_default
style_home_school_img_main_main_default = lv.style_t()
style_home_school_img_main_main_default.init()
style_home_school_img_main_main_default.set_img_recolor(lv.color_make(0x00,0x01,0xff))
style_home_school_img_main_main_default.set_img_recolor_opa(0)
style_home_school_img_main_main_default.set_img_opa(255)

# add style for home_school_img
home_school_img.add_style(style_home_school_img_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create home_video_btn
home_video_btn = lv.imgbtn(home)
home_video_btn.set_pos(int(418),int(519))
home_video_btn.set_size(70,70)
home_video_btn.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
try:
    with open('D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp1174212390.png','rb') as f:
        home_video_btn_imgReleased_data = f.read()
except:
    print('Could not open D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp1174212390.png')
    sys.exit()

home_video_btn_imgReleased = lv.img_dsc_t({
  'data_size': len(home_video_btn_imgReleased_data),
  'header': {'always_zero': 0, 'w': 70, 'h': 70, 'cf': lv.img.CF.TRUE_COLOR_ALPHA},
  'data': home_video_btn_imgReleased_data
})
home_video_btn.set_src(lv.imgbtn.STATE.RELEASED, None, home_video_btn_imgReleased, None)





home_video_btn.add_flag(lv.obj.FLAG.CHECKABLE)
# create style style_home_video_btn_main_main_default
style_home_video_btn_main_main_default = lv.style_t()
style_home_video_btn_main_main_default.init()
style_home_video_btn_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_home_video_btn_main_main_default.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_home_video_btn_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_home_video_btn_main_main_default.set_text_font(lv.font_montserrat_16)
style_home_video_btn_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_home_video_btn_main_main_default.set_img_recolor(lv.color_make(0xff,0xff,0xff))
style_home_video_btn_main_main_default.set_img_recolor_opa(0)
style_home_video_btn_main_main_default.set_img_opa(255)

# add style for home_video_btn
home_video_btn.add_style(style_home_video_btn_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_home_video_btn_main_main_pressed
style_home_video_btn_main_main_pressed = lv.style_t()
style_home_video_btn_main_main_pressed.init()
style_home_video_btn_main_main_pressed.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_home_video_btn_main_main_pressed.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_home_video_btn_main_main_pressed.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_home_video_btn_main_main_pressed.set_text_font(lv.font_montserrat_16)
style_home_video_btn_main_main_pressed.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_home_video_btn_main_main_pressed.set_img_recolor_opa(0)
style_home_video_btn_main_main_pressed.set_img_opa(255)

# add style for home_video_btn
home_video_btn.add_style(style_home_video_btn_main_main_pressed, lv.PART.MAIN|lv.STATE.PRESSED)

# create style style_home_video_btn_main_main_checked
style_home_video_btn_main_main_checked = lv.style_t()
style_home_video_btn_main_main_checked.init()
style_home_video_btn_main_main_checked.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_home_video_btn_main_main_checked.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_home_video_btn_main_main_checked.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_home_video_btn_main_main_checked.set_text_font(lv.font_montserrat_16)
style_home_video_btn_main_main_checked.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_home_video_btn_main_main_checked.set_img_recolor_opa(0)
style_home_video_btn_main_main_checked.set_img_opa(255)

# add style for home_video_btn
home_video_btn.add_style(style_home_video_btn_main_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)


# create home_backup_btn
home_backup_btn = lv.imgbtn(home)
home_backup_btn.set_pos(int(538),int(520))
home_backup_btn.set_size(70,70)
home_backup_btn.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
try:
    with open('D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp581698822.png','rb') as f:
        home_backup_btn_imgReleased_data = f.read()
except:
    print('Could not open D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp581698822.png')
    sys.exit()

home_backup_btn_imgReleased = lv.img_dsc_t({
  'data_size': len(home_backup_btn_imgReleased_data),
  'header': {'always_zero': 0, 'w': 70, 'h': 70, 'cf': lv.img.CF.TRUE_COLOR_ALPHA},
  'data': home_backup_btn_imgReleased_data
})
home_backup_btn.set_src(lv.imgbtn.STATE.RELEASED, None, home_backup_btn_imgReleased, None)





home_backup_btn.add_flag(lv.obj.FLAG.CHECKABLE)
# create style style_home_backup_btn_main_main_default
style_home_backup_btn_main_main_default = lv.style_t()
style_home_backup_btn_main_main_default.init()
style_home_backup_btn_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_home_backup_btn_main_main_default.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_home_backup_btn_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_home_backup_btn_main_main_default.set_text_font(lv.font_montserrat_16)
style_home_backup_btn_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_home_backup_btn_main_main_default.set_img_recolor(lv.color_make(0xff,0xff,0xff))
style_home_backup_btn_main_main_default.set_img_recolor_opa(0)
style_home_backup_btn_main_main_default.set_img_opa(255)

# add style for home_backup_btn
home_backup_btn.add_style(style_home_backup_btn_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_home_backup_btn_main_main_pressed
style_home_backup_btn_main_main_pressed = lv.style_t()
style_home_backup_btn_main_main_pressed.init()
style_home_backup_btn_main_main_pressed.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_home_backup_btn_main_main_pressed.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_home_backup_btn_main_main_pressed.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_home_backup_btn_main_main_pressed.set_text_font(lv.font_montserrat_16)
style_home_backup_btn_main_main_pressed.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_home_backup_btn_main_main_pressed.set_img_recolor_opa(0)
style_home_backup_btn_main_main_pressed.set_img_opa(255)

# add style for home_backup_btn
home_backup_btn.add_style(style_home_backup_btn_main_main_pressed, lv.PART.MAIN|lv.STATE.PRESSED)

# create style style_home_backup_btn_main_main_checked
style_home_backup_btn_main_main_checked = lv.style_t()
style_home_backup_btn_main_main_checked.init()
style_home_backup_btn_main_main_checked.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_home_backup_btn_main_main_checked.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_home_backup_btn_main_main_checked.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_home_backup_btn_main_main_checked.set_text_font(lv.font_montserrat_16)
style_home_backup_btn_main_main_checked.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_home_backup_btn_main_main_checked.set_img_recolor_opa(0)
style_home_backup_btn_main_main_checked.set_img_opa(255)

# add style for home_backup_btn
home_backup_btn.add_style(style_home_backup_btn_main_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)


# create home_car_img
home_car_img = lv.img(home)
home_car_img.set_pos(int(183),int(122))
home_car_img.set_size(659,322)
home_car_img.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
home_car_img.add_flag(lv.obj.FLAG.CLICKABLE)
try:
    with open('D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp1941367910.png','rb') as f:
        home_car_img_img_data = f.read()
except:
    print('Could not open D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp1941367910.png')
    sys.exit()

home_car_img_img = lv.img_dsc_t({
  'data_size': len(home_car_img_img_data),
  'header': {'always_zero': 0, 'w': 659, 'h': 322, 'cf': lv.img.CF.TRUE_COLOR_ALPHA},
  'data': home_car_img_img_data
})

home_car_img.set_src(home_car_img_img)
home_car_img.set_pivot(50,50)
home_car_img.set_angle(0)
# create style style_home_car_img_main_main_default
style_home_car_img_main_main_default = lv.style_t()
style_home_car_img_main_main_default.init()
style_home_car_img_main_main_default.set_img_recolor(lv.color_make(0xff,0xff,0xff))
style_home_car_img_main_main_default.set_img_recolor_opa(0)
style_home_car_img_main_main_default.set_img_opa(255)

# add style for home_car_img
home_car_img.add_style(style_home_car_img_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create home_spangroup_1
home_spangroup_1 = lv.spangroup(home)
home_spangroup_1.set_pos(int(48),int(48))
home_spangroup_1.set_size(86,20)
home_spangroup_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
home_spangroup_1.set_align(lv.TEXT_ALIGN.LEFT)
home_spangroup_1.set_overflow(lv.SPAN_OVERFLOW.CLIP)
home_spangroup_1.set_mode(lv.SPAN_MODE.BREAK)
home_spangroup_1_span = home_spangroup_1.new_span()
home_spangroup_1_span.set_text("星期四")
home_spangroup_1_span.style.set_text_color(lv.color_make(0xff,0xff,0xff))
home_spangroup_1_span.style.set_text_decor(lv.TEXT_DECOR.NONE)
try:
    home_spangroup_1_span.style.set_text_font(lv.font_simsun_20)
except AttributeError:
    try:
        home_spangroup_1_span.style.set_text_font(lv.font_montserrat_20)
    except AttributeError:
        home_spangroup_1_span.style.set_text_font(lv.font_montserrat_16)
home_spangroup_1.refr_mode()
# create style style_home_spangroup_1_main_main_default
style_home_spangroup_1_main_main_default = lv.style_t()
style_home_spangroup_1_main_main_default.init()
style_home_spangroup_1_main_main_default.set_radius(0)
style_home_spangroup_1_main_main_default.set_bg_color(lv.color_make(0xfb,0xb6,0xb6))
style_home_spangroup_1_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_home_spangroup_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_home_spangroup_1_main_main_default.set_bg_opa(0)
style_home_spangroup_1_main_main_default.set_border_color(lv.color_make(0x00,0x00,0x00))
style_home_spangroup_1_main_main_default.set_border_width(0)
style_home_spangroup_1_main_main_default.set_border_opa(255)
style_home_spangroup_1_main_main_default.set_pad_left(0)
style_home_spangroup_1_main_main_default.set_pad_right(0)
style_home_spangroup_1_main_main_default.set_pad_top(0)
style_home_spangroup_1_main_main_default.set_pad_bottom(0)

# add style for home_spangroup_1
home_spangroup_1.add_style(style_home_spangroup_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create music
music = lv.obj()
music.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_music_main_main_default
style_music_main_main_default = lv.style_t()
style_music_main_main_default.init()
style_music_main_main_default.set_bg_color(lv.color_make(0xf5,0xef,0xf0))
style_music_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_music_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_music_main_main_default.set_bg_opa(255)

# add style for music
music.add_style(style_music_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create music_music_return_btn
music_music_return_btn = lv.imgbtn(music)
music_music_return_btn.set_pos(int(920),int(45))
music_music_return_btn.set_size(50,50)
music_music_return_btn.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
try:
    with open('D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp-1291335315.png','rb') as f:
        music_music_return_btn_imgReleased_data = f.read()
except:
    print('Could not open D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp-1291335315.png')
    sys.exit()

music_music_return_btn_imgReleased = lv.img_dsc_t({
  'data_size': len(music_music_return_btn_imgReleased_data),
  'header': {'always_zero': 0, 'w': 50, 'h': 50, 'cf': lv.img.CF.TRUE_COLOR_ALPHA},
  'data': music_music_return_btn_imgReleased_data
})
music_music_return_btn.set_src(lv.imgbtn.STATE.RELEASED, None, music_music_return_btn_imgReleased, None)





music_music_return_btn.add_flag(lv.obj.FLAG.CHECKABLE)
# create style style_music_music_return_btn_main_main_default
style_music_music_return_btn_main_main_default = lv.style_t()
style_music_music_return_btn_main_main_default.init()
style_music_music_return_btn_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_music_music_return_btn_main_main_default.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_music_music_return_btn_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_music_music_return_btn_main_main_default.set_text_font(lv.font_montserrat_16)
style_music_music_return_btn_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_music_music_return_btn_main_main_default.set_img_recolor(lv.color_make(0xff,0xff,0xff))
style_music_music_return_btn_main_main_default.set_img_recolor_opa(0)
style_music_music_return_btn_main_main_default.set_img_opa(255)

# add style for music_music_return_btn
music_music_return_btn.add_style(style_music_music_return_btn_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_music_music_return_btn_main_main_pressed
style_music_music_return_btn_main_main_pressed = lv.style_t()
style_music_music_return_btn_main_main_pressed.init()
style_music_music_return_btn_main_main_pressed.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_music_music_return_btn_main_main_pressed.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_music_music_return_btn_main_main_pressed.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_music_music_return_btn_main_main_pressed.set_text_font(lv.font_montserrat_16)
style_music_music_return_btn_main_main_pressed.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_music_music_return_btn_main_main_pressed.set_img_recolor_opa(0)
style_music_music_return_btn_main_main_pressed.set_img_opa(255)

# add style for music_music_return_btn
music_music_return_btn.add_style(style_music_music_return_btn_main_main_pressed, lv.PART.MAIN|lv.STATE.PRESSED)

# create style style_music_music_return_btn_main_main_checked
style_music_music_return_btn_main_main_checked = lv.style_t()
style_music_music_return_btn_main_main_checked.init()
style_music_music_return_btn_main_main_checked.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_music_music_return_btn_main_main_checked.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_music_music_return_btn_main_main_checked.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_music_music_return_btn_main_main_checked.set_text_font(lv.font_montserrat_16)
style_music_music_return_btn_main_main_checked.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_music_music_return_btn_main_main_checked.set_img_recolor_opa(0)
style_music_music_return_btn_main_main_checked.set_img_opa(255)

# add style for music_music_return_btn
music_music_return_btn.add_style(style_music_music_return_btn_main_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)


# create video
video = lv.obj()
video.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_video_main_main_default
style_video_main_main_default = lv.style_t()
style_video_main_main_default.init()
style_video_main_main_default.set_bg_color(lv.color_make(0xff,0xff,0xff))
style_video_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_video_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_video_main_main_default.set_bg_opa(235)

# add style for video
video.add_style(style_video_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create video_imgbtn_1
video_imgbtn_1 = lv.imgbtn(video)
video_imgbtn_1.set_pos(int(920),int(45))
video_imgbtn_1.set_size(50,50)
video_imgbtn_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
try:
    with open('D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp-1291335315.png','rb') as f:
        video_imgbtn_1_imgReleased_data = f.read()
except:
    print('Could not open D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp-1291335315.png')
    sys.exit()

video_imgbtn_1_imgReleased = lv.img_dsc_t({
  'data_size': len(video_imgbtn_1_imgReleased_data),
  'header': {'always_zero': 0, 'w': 50, 'h': 50, 'cf': lv.img.CF.TRUE_COLOR_ALPHA},
  'data': video_imgbtn_1_imgReleased_data
})
video_imgbtn_1.set_src(lv.imgbtn.STATE.RELEASED, None, video_imgbtn_1_imgReleased, None)





video_imgbtn_1.add_flag(lv.obj.FLAG.CHECKABLE)
# create style style_video_imgbtn_1_main_main_default
style_video_imgbtn_1_main_main_default = lv.style_t()
style_video_imgbtn_1_main_main_default.init()
style_video_imgbtn_1_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_video_imgbtn_1_main_main_default.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_video_imgbtn_1_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_video_imgbtn_1_main_main_default.set_text_font(lv.font_montserrat_16)
style_video_imgbtn_1_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_video_imgbtn_1_main_main_default.set_img_recolor(lv.color_make(0xff,0xff,0xff))
style_video_imgbtn_1_main_main_default.set_img_recolor_opa(0)
style_video_imgbtn_1_main_main_default.set_img_opa(255)

# add style for video_imgbtn_1
video_imgbtn_1.add_style(style_video_imgbtn_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_video_imgbtn_1_main_main_pressed
style_video_imgbtn_1_main_main_pressed = lv.style_t()
style_video_imgbtn_1_main_main_pressed.init()
style_video_imgbtn_1_main_main_pressed.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_video_imgbtn_1_main_main_pressed.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_video_imgbtn_1_main_main_pressed.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_video_imgbtn_1_main_main_pressed.set_text_font(lv.font_montserrat_16)
style_video_imgbtn_1_main_main_pressed.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_video_imgbtn_1_main_main_pressed.set_img_recolor_opa(0)
style_video_imgbtn_1_main_main_pressed.set_img_opa(255)

# add style for video_imgbtn_1
video_imgbtn_1.add_style(style_video_imgbtn_1_main_main_pressed, lv.PART.MAIN|lv.STATE.PRESSED)

# create style style_video_imgbtn_1_main_main_checked
style_video_imgbtn_1_main_main_checked = lv.style_t()
style_video_imgbtn_1_main_main_checked.init()
style_video_imgbtn_1_main_main_checked.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_video_imgbtn_1_main_main_checked.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_video_imgbtn_1_main_main_checked.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_video_imgbtn_1_main_main_checked.set_text_font(lv.font_montserrat_16)
style_video_imgbtn_1_main_main_checked.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_video_imgbtn_1_main_main_checked.set_img_recolor_opa(0)
style_video_imgbtn_1_main_main_checked.set_img_opa(255)

# add style for video_imgbtn_1
video_imgbtn_1.add_style(style_video_imgbtn_1_main_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)


# create back_up
back_up = lv.obj()
back_up.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_back_up_main_main_default
style_back_up_main_main_default = lv.style_t()
style_back_up_main_main_default.init()
style_back_up_main_main_default.set_bg_color(lv.color_make(0xff,0xff,0xff))
style_back_up_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_back_up_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_back_up_main_main_default.set_bg_opa(242)

# add style for back_up
back_up.add_style(style_back_up_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create back_up_imgbtn_2
back_up_imgbtn_2 = lv.imgbtn(back_up)
back_up_imgbtn_2.set_pos(int(920),int(45))
back_up_imgbtn_2.set_size(50,50)
back_up_imgbtn_2.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
try:
    with open('D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp-1291335315.png','rb') as f:
        back_up_imgbtn_2_imgReleased_data = f.read()
except:
    print('Could not open D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp-1291335315.png')
    sys.exit()

back_up_imgbtn_2_imgReleased = lv.img_dsc_t({
  'data_size': len(back_up_imgbtn_2_imgReleased_data),
  'header': {'always_zero': 0, 'w': 50, 'h': 50, 'cf': lv.img.CF.TRUE_COLOR_ALPHA},
  'data': back_up_imgbtn_2_imgReleased_data
})
back_up_imgbtn_2.set_src(lv.imgbtn.STATE.RELEASED, None, back_up_imgbtn_2_imgReleased, None)





back_up_imgbtn_2.add_flag(lv.obj.FLAG.CHECKABLE)
# create style style_back_up_imgbtn_2_main_main_default
style_back_up_imgbtn_2_main_main_default = lv.style_t()
style_back_up_imgbtn_2_main_main_default.init()
style_back_up_imgbtn_2_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_back_up_imgbtn_2_main_main_default.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_back_up_imgbtn_2_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_back_up_imgbtn_2_main_main_default.set_text_font(lv.font_montserrat_16)
style_back_up_imgbtn_2_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_back_up_imgbtn_2_main_main_default.set_img_recolor(lv.color_make(0xff,0xff,0xff))
style_back_up_imgbtn_2_main_main_default.set_img_recolor_opa(0)
style_back_up_imgbtn_2_main_main_default.set_img_opa(255)

# add style for back_up_imgbtn_2
back_up_imgbtn_2.add_style(style_back_up_imgbtn_2_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_back_up_imgbtn_2_main_main_pressed
style_back_up_imgbtn_2_main_main_pressed = lv.style_t()
style_back_up_imgbtn_2_main_main_pressed.init()
style_back_up_imgbtn_2_main_main_pressed.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_back_up_imgbtn_2_main_main_pressed.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_back_up_imgbtn_2_main_main_pressed.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_back_up_imgbtn_2_main_main_pressed.set_text_font(lv.font_montserrat_16)
style_back_up_imgbtn_2_main_main_pressed.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_back_up_imgbtn_2_main_main_pressed.set_img_recolor_opa(0)
style_back_up_imgbtn_2_main_main_pressed.set_img_opa(255)

# add style for back_up_imgbtn_2
back_up_imgbtn_2.add_style(style_back_up_imgbtn_2_main_main_pressed, lv.PART.MAIN|lv.STATE.PRESSED)

# create style style_back_up_imgbtn_2_main_main_checked
style_back_up_imgbtn_2_main_main_checked = lv.style_t()
style_back_up_imgbtn_2_main_main_checked.init()
style_back_up_imgbtn_2_main_main_checked.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_back_up_imgbtn_2_main_main_checked.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_back_up_imgbtn_2_main_main_checked.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_back_up_imgbtn_2_main_main_checked.set_text_font(lv.font_montserrat_16)
style_back_up_imgbtn_2_main_main_checked.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_back_up_imgbtn_2_main_main_checked.set_img_recolor_opa(0)
style_back_up_imgbtn_2_main_main_checked.set_img_opa(255)

# add style for back_up_imgbtn_2
back_up_imgbtn_2.add_style(style_back_up_imgbtn_2_main_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)


# create setting
setting = lv.obj()
setting.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_setting_main_main_default
style_setting_main_main_default = lv.style_t()
style_setting_main_main_default.init()
style_setting_main_main_default.set_bg_color(lv.color_make(0xff,0xf5,0xf5))
style_setting_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_setting_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_setting_main_main_default.set_bg_opa(255)

# add style for setting
setting.add_style(style_setting_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create setting_setting_return_btn
setting_setting_return_btn = lv.imgbtn(setting)
setting_setting_return_btn.set_pos(int(920),int(45))
setting_setting_return_btn.set_size(50,50)
setting_setting_return_btn.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
try:
    with open('D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp-1291335315.png','rb') as f:
        setting_setting_return_btn_imgReleased_data = f.read()
except:
    print('Could not open D:\\06_Electronic\\NXP\\GUI-Guider-Projects\\my_graduation_project\\generated\\mPythonImages\\mp-1291335315.png')
    sys.exit()

setting_setting_return_btn_imgReleased = lv.img_dsc_t({
  'data_size': len(setting_setting_return_btn_imgReleased_data),
  'header': {'always_zero': 0, 'w': 50, 'h': 50, 'cf': lv.img.CF.TRUE_COLOR_ALPHA},
  'data': setting_setting_return_btn_imgReleased_data
})
setting_setting_return_btn.set_src(lv.imgbtn.STATE.RELEASED, None, setting_setting_return_btn_imgReleased, None)





setting_setting_return_btn.add_flag(lv.obj.FLAG.CHECKABLE)
# create style style_setting_setting_return_btn_main_main_default
style_setting_setting_return_btn_main_main_default = lv.style_t()
style_setting_setting_return_btn_main_main_default.init()
style_setting_setting_return_btn_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_setting_setting_return_btn_main_main_default.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_setting_setting_return_btn_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_setting_setting_return_btn_main_main_default.set_text_font(lv.font_montserrat_16)
style_setting_setting_return_btn_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_setting_setting_return_btn_main_main_default.set_img_recolor(lv.color_make(0xff,0xff,0xff))
style_setting_setting_return_btn_main_main_default.set_img_recolor_opa(0)
style_setting_setting_return_btn_main_main_default.set_img_opa(255)

# add style for setting_setting_return_btn
setting_setting_return_btn.add_style(style_setting_setting_return_btn_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_setting_setting_return_btn_main_main_pressed
style_setting_setting_return_btn_main_main_pressed = lv.style_t()
style_setting_setting_return_btn_main_main_pressed.init()
style_setting_setting_return_btn_main_main_pressed.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_setting_setting_return_btn_main_main_pressed.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_setting_setting_return_btn_main_main_pressed.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_setting_setting_return_btn_main_main_pressed.set_text_font(lv.font_montserrat_16)
style_setting_setting_return_btn_main_main_pressed.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_setting_setting_return_btn_main_main_pressed.set_img_recolor_opa(0)
style_setting_setting_return_btn_main_main_pressed.set_img_opa(255)

# add style for setting_setting_return_btn
setting_setting_return_btn.add_style(style_setting_setting_return_btn_main_main_pressed, lv.PART.MAIN|lv.STATE.PRESSED)

# create style style_setting_setting_return_btn_main_main_checked
style_setting_setting_return_btn_main_main_checked = lv.style_t()
style_setting_setting_return_btn_main_main_checked.init()
style_setting_setting_return_btn_main_main_checked.set_text_color(lv.color_make(0xFF,0x33,0xFF))
try:
    style_setting_setting_return_btn_main_main_checked.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_setting_setting_return_btn_main_main_checked.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_setting_setting_return_btn_main_main_checked.set_text_font(lv.font_montserrat_16)
style_setting_setting_return_btn_main_main_checked.set_img_recolor(lv.color_make(0x00,0x00,0x00))
style_setting_setting_return_btn_main_main_checked.set_img_recolor_opa(0)
style_setting_setting_return_btn_main_main_checked.set_img_opa(255)

# add style for setting_setting_return_btn
setting_setting_return_btn.add_style(style_setting_setting_return_btn_main_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)


# create setting_menu_1
setting_menu_1 = lv.menu(setting)
setting_menu_1.set_pos(int(0),int(40))
setting_menu_1.set_size(458,171)
setting_menu_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_setting_menu_1_extra_main_page_main_default
style_setting_menu_1_extra_main_page_main_default = lv.style_t()
style_setting_menu_1_extra_main_page_main_default.init()
style_setting_menu_1_extra_main_page_main_default.set_radius(0)
style_setting_menu_1_extra_main_page_main_default.set_bg_color(lv.color_make(0xda,0xf2,0xf8))
style_setting_menu_1_extra_main_page_main_default.set_bg_grad_color(lv.color_make(0x5a,0x61,0x73))
style_setting_menu_1_extra_main_page_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_setting_menu_1_extra_main_page_main_default.set_bg_opa(255)


# create style style_setting_menu_1_extra_option_btns_main_default
style_setting_menu_1_extra_option_btns_main_default = lv.style_t()
style_setting_menu_1_extra_option_btns_main_default.init()
style_setting_menu_1_extra_option_btns_main_default.set_text_color(lv.color_make(0x15,0x12,0x12))
try:
    style_setting_menu_1_extra_option_btns_main_default.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_setting_menu_1_extra_option_btns_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_setting_menu_1_extra_option_btns_main_default.set_text_font(lv.font_montserrat_16)


# create style style_setting_menu_1_extra_option_btns_main_checked
style_setting_menu_1_extra_option_btns_main_checked = lv.style_t()
style_setting_menu_1_extra_option_btns_main_checked.init()
style_setting_menu_1_extra_option_btns_main_checked.set_radius(0)
style_setting_menu_1_extra_option_btns_main_checked.set_bg_color(lv.color_make(0x00,0xe0,0xb8))
style_setting_menu_1_extra_option_btns_main_checked.set_bg_grad_color(lv.color_make(0x32,0x9d,0xf6))
style_setting_menu_1_extra_option_btns_main_checked.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_setting_menu_1_extra_option_btns_main_checked.set_bg_opa(60)
style_setting_menu_1_extra_option_btns_main_checked.set_border_color(lv.color_make(0x32,0x9d,0xf6))
style_setting_menu_1_extra_option_btns_main_checked.set_border_width(0)
style_setting_menu_1_extra_option_btns_main_checked.set_border_opa(255)
style_setting_menu_1_extra_option_btns_main_checked.set_border_side(lv.BORDER_SIDE.BOTTOM)
style_setting_menu_1_extra_option_btns_main_checked.set_text_color(lv.color_make(0x9a,0xb7,0x00))
try:
    style_setting_menu_1_extra_option_btns_main_checked.set_text_font(lv.font_montserratMedium_12)
except AttributeError:
    try:
        style_setting_menu_1_extra_option_btns_main_checked.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_setting_menu_1_extra_option_btns_main_checked.set_text_font(lv.font_montserrat_16)



setting_menu_1_main_page = lv.menu_page(setting_menu_1, None)



setting_menu_1_sub_page0 = lv.menu_page(setting_menu_1, None)


# add style for setting_menu_1_main_page
setting_menu_1_main_page.add_style(style_setting_menu_1_extra_main_page_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

setting_menu_1_menu_cont0 = lv.menu_cont(setting_menu_1_main_page)

# add style for setting_menu_1_menu_cont0
setting_menu_1_menu_cont0.add_style(style_setting_menu_1_extra_option_btns_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# add style for setting_menu_1_menu_cont0
setting_menu_1_menu_cont0.add_style(style_setting_menu_1_extra_option_btns_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)

setting_menu_1_menu_label0 = lv.label(setting_menu_1_menu_cont0)
setting_menu_1_menu_label0.set_text("menu_4")
setting_menu_1.set_load_page_event(setting_menu_1_menu_cont0, setting_menu_1_sub_page0)

setting_menu_1_sub_page1 = lv.menu_page(setting_menu_1, None)


# add style for setting_menu_1_menu_cont0
setting_menu_1_menu_cont0.add_style(style_setting_menu_1_extra_option_btns_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# add style for setting_menu_1_menu_cont0
setting_menu_1_menu_cont0.add_style(style_setting_menu_1_extra_option_btns_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)

setting_menu_1_menu_cont1 = lv.menu_cont(setting_menu_1_main_page)

# add style for setting_menu_1_menu_cont1
setting_menu_1_menu_cont1.add_style(style_setting_menu_1_extra_option_btns_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# add style for setting_menu_1_menu_cont1
setting_menu_1_menu_cont1.add_style(style_setting_menu_1_extra_option_btns_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)

setting_menu_1_menu_label1 = lv.label(setting_menu_1_menu_cont1)
setting_menu_1_menu_label1.set_text("TIME_DATE")
setting_menu_1.set_load_page_event(setting_menu_1_menu_cont1, setting_menu_1_sub_page1)

setting_menu_1_sub_page2 = lv.menu_page(setting_menu_1, None)


# add style for setting_menu_1_menu_cont1
setting_menu_1_menu_cont1.add_style(style_setting_menu_1_extra_option_btns_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# add style for setting_menu_1_menu_cont1
setting_menu_1_menu_cont1.add_style(style_setting_menu_1_extra_option_btns_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)

setting_menu_1_menu_cont2 = lv.menu_cont(setting_menu_1_main_page)

# add style for setting_menu_1_menu_cont2
setting_menu_1_menu_cont2.add_style(style_setting_menu_1_extra_option_btns_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# add style for setting_menu_1_menu_cont2
setting_menu_1_menu_cont2.add_style(style_setting_menu_1_extra_option_btns_main_checked, lv.PART.MAIN|lv.STATE.CHECKED)

setting_menu_1_menu_label2 = lv.label(setting_menu_1_menu_cont2)
setting_menu_1_menu_label2.set_text("LUMINANCE")
setting_menu_1.set_load_page_event(setting_menu_1_menu_cont2, setting_menu_1_sub_page2)


setting_menu_1.set_sidebar_page(setting_menu_1_main_page)

# create style style_setting_menu_1_main_main_default
style_setting_menu_1_main_main_default = lv.style_t()
style_setting_menu_1_main_main_default.init()
style_setting_menu_1_main_main_default.set_radius(10)
style_setting_menu_1_main_main_default.set_bg_color(lv.color_make(0xff,0xff,0xff))
style_setting_menu_1_main_main_default.set_bg_grad_color(lv.color_make(0x41,0x48,0x5a))
style_setting_menu_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_setting_menu_1_main_main_default.set_bg_opa(255)

# add style for setting_menu_1
setting_menu_1.add_style(style_setting_menu_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_setting_menu_1_main_main_default
style_setting_menu_1_main_main_default = lv.style_t()
style_setting_menu_1_main_main_default.init()
style_setting_menu_1_main_main_default.set_radius(10)
style_setting_menu_1_main_main_default.set_bg_color(lv.color_make(0xff,0xff,0xff))
style_setting_menu_1_main_main_default.set_bg_grad_color(lv.color_make(0x41,0x48,0x5a))
style_setting_menu_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_setting_menu_1_main_main_default.set_bg_opa(255)

# add style for setting_menu_1
setting_menu_1.add_style(style_setting_menu_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create setting_slider_1
setting_slider_1 = lv.slider(setting)
setting_slider_1.set_pos(int(530),int(160))
setting_slider_1.set_size(309,36)
setting_slider_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
setting_slider_1.set_range(0, 100)
setting_slider_1.set_value(50, False)

# create style style_setting_slider_1_main_main_default
style_setting_slider_1_main_main_default = lv.style_t()
style_setting_slider_1_main_main_default.init()
style_setting_slider_1_main_main_default.set_radius(50)
style_setting_slider_1_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_setting_slider_1_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_setting_slider_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_setting_slider_1_main_main_default.set_bg_opa(60)
style_setting_slider_1_main_main_default.set_outline_color(lv.color_make(0x21,0x95,0xf6))
style_setting_slider_1_main_main_default.set_outline_width(0)
style_setting_slider_1_main_main_default.set_outline_opa(0)

# add style for setting_slider_1
setting_slider_1.add_style(style_setting_slider_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_setting_slider_1_main_indicator_default
style_setting_slider_1_main_indicator_default = lv.style_t()
style_setting_slider_1_main_indicator_default.init()
style_setting_slider_1_main_indicator_default.set_radius(50)
style_setting_slider_1_main_indicator_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_setting_slider_1_main_indicator_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_setting_slider_1_main_indicator_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_setting_slider_1_main_indicator_default.set_bg_opa(255)

# add style for setting_slider_1
setting_slider_1.add_style(style_setting_slider_1_main_indicator_default, lv.PART.INDICATOR|lv.STATE.DEFAULT)

# create style style_setting_slider_1_main_knob_default
style_setting_slider_1_main_knob_default = lv.style_t()
style_setting_slider_1_main_knob_default.init()
style_setting_slider_1_main_knob_default.set_radius(50)
style_setting_slider_1_main_knob_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_setting_slider_1_main_knob_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_setting_slider_1_main_knob_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_setting_slider_1_main_knob_default.set_bg_opa(255)

# add style for setting_slider_1
setting_slider_1.add_style(style_setting_slider_1_main_knob_default, lv.PART.KNOB|lv.STATE.DEFAULT)


# create setting_textprogress_1
setting_textprogress_1 = lv.label(setting)
setting_textprogress_1.set_style_text_align(lv.TEXT_ALIGN.CENTER, 0)

def setting_textprogress_1_muldiv(a, b, c):
    return int((a*b)/c)

def setting_textprogress_1_get_progress(range, inital_value):
    setting_textprogress_1_range_min = 0
    setting_textprogress_1_range_max = 100
    setting_textprogress_1_range_steps_min = 0
    setting_textprogress_1_range_steps = 0
    setting_textprogress_1_inital_value = inital_value
    if setting_textprogress_1_range_steps == 0:
       setting_textprogress_1_range_steps = setting_textprogress_1_range_max - setting_textprogress_1_range_min
    step = setting_textprogress_1_range_steps_min + setting_textprogress_1_muldiv(((setting_textprogress_1_inital_value)-(setting_textprogress_1_range_min)), ((setting_textprogress_1_range_steps)-(setting_textprogress_1_range_steps_min)), (setting_textprogress_1_range_max-setting_textprogress_1_range_min))
    prog = setting_textprogress_1_muldiv(step, range, setting_textprogress_1_range_steps)
    return int(prog)

def setting_textprogress_1_get_range(decimals):
    setting_textprogress_1_decimals = decimals
    if setting_textprogress_1_decimals == 0:
       setting_textprogress_1_range = 1
    else:
      if setting_textprogress_1_decimals == 1:
         setting_textprogress_1_range = 10
      else:
        if setting_textprogress_1_decimals == 2:
           setting_textprogress_1_range = 100
    return setting_textprogress_1_range

def setting_textprogress_1_set_text(inital_value, decimals):
    setting_textprogress_1_decimals = decimals
    setting_textprogress_1_range = setting_textprogress_1_get_range(setting_textprogress_1_decimals)
    progress = setting_textprogress_1_get_progress( 100 *setting_textprogress_1_range, inital_value)
    if setting_textprogress_1_decimals > 0:
       setting_textprogress_1.set_text("%d.%0*d%% "%((progress/setting_textprogress_1_range), setting_textprogress_1_decimals, (progress%setting_textprogress_1_range)))
    else:
       setting_textprogress_1.set_text("%d%%"%(progress))

setting_textprogress_1_set_text(80, 0)

setting_textprogress_1.set_pos(int(876),int(164))
setting_textprogress_1.set_size(100,32)
setting_textprogress_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_setting_textprogress_1_main_main_default
style_setting_textprogress_1_main_main_default = lv.style_t()
style_setting_textprogress_1_main_main_default.init()
style_setting_textprogress_1_main_main_default.set_radius(0)
style_setting_textprogress_1_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_setting_textprogress_1_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_setting_textprogress_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.VER)
style_setting_textprogress_1_main_main_default.set_bg_opa(255)
style_setting_textprogress_1_main_main_default.set_text_color(lv.color_make(0xff,0xff,0xff))
try:
    style_setting_textprogress_1_main_main_default.set_text_font(lv.font_simsun_16)
except AttributeError:
    try:
        style_setting_textprogress_1_main_main_default.set_text_font(lv.font_montserrat_16)
    except AttributeError:
        style_setting_textprogress_1_main_main_default.set_text_font(lv.font_montserrat_16)
style_setting_textprogress_1_main_main_default.set_text_letter_space(2)
style_setting_textprogress_1_main_main_default.set_pad_left(0)
style_setting_textprogress_1_main_main_default.set_pad_right(0)
style_setting_textprogress_1_main_main_default.set_pad_top(8)
style_setting_textprogress_1_main_main_default.set_pad_bottom(0)

# add style for setting_textprogress_1
setting_textprogress_1.add_style(style_setting_textprogress_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)


# create setting_carousel_1
setting_carousel_1 = lv.carousel(setting)
setting_carousel_1.set_pos(int(282),int(335))
setting_carousel_1.set_size(310,200)
setting_carousel_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
setting_carousel_1.set_element_width(100)

caro_3 = setting_carousel_1.add_element(1)
caro_2 = setting_carousel_1.add_element(0)
# create style style_setting_carousel_1_main_main_default
style_setting_carousel_1_main_main_default = lv.style_t()
style_setting_carousel_1_main_main_default.init()
style_setting_carousel_1_main_main_default.set_radius(0)
style_setting_carousel_1_main_main_default.set_bg_color(lv.color_make(0xff,0xff,0xff))
style_setting_carousel_1_main_main_default.set_bg_grad_color(lv.color_make(0xff,0xff,0xff))
style_setting_carousel_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_setting_carousel_1_main_main_default.set_bg_opa(255)

# add style for setting_carousel_1
setting_carousel_1.add_style(style_setting_carousel_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_setting_carousel_1_main_scrollbar_default
style_setting_carousel_1_main_scrollbar_default = lv.style_t()
style_setting_carousel_1_main_scrollbar_default.init()
style_setting_carousel_1_main_scrollbar_default.set_radius(0)
style_setting_carousel_1_main_scrollbar_default.set_bg_color(lv.color_make(0xcc,0xcc,0xcc))
style_setting_carousel_1_main_scrollbar_default.set_bg_opa(255)

# add style for setting_carousel_1
setting_carousel_1.add_style(style_setting_carousel_1_main_scrollbar_default, lv.PART.SCROLLBAR|lv.STATE.DEFAULT)



def home_setting_btn_clicked_1_event_cb(e,setting):
    src = e.get_target()
    code = e.get_code()
    lv.scr_load_anim(setting, lv.SCR_LOAD_ANIM.OVER_LEFT, 100, 100, False)
home_setting_btn.add_event_cb(lambda e: home_setting_btn_clicked_1_event_cb(e,setting), lv.EVENT.CLICKED, None)


def back_up_imgbtn_2_clicked_1_event_cb(e,home):
    src = e.get_target()
    code = e.get_code()
    lv.scr_load_anim(home, lv.SCR_LOAD_ANIM.OVER_RIGHT, 100, 100, False)
back_up_imgbtn_2.add_event_cb(lambda e: back_up_imgbtn_2_clicked_1_event_cb(e,home), lv.EVENT.CLICKED, None)


def setting_setting_return_btn_clicked_1_event_cb(e,home):
    src = e.get_target()
    code = e.get_code()
    lv.scr_load_anim(home, lv.SCR_LOAD_ANIM.OVER_RIGHT, 100, 100, False)
setting_setting_return_btn.add_event_cb(lambda e: setting_setting_return_btn_clicked_1_event_cb(e,home), lv.EVENT.CLICKED, None)


def home_video_btn_clicked_1_event_cb(e,video):
    src = e.get_target()
    code = e.get_code()
    lv.scr_load_anim(video, lv.SCR_LOAD_ANIM.OVER_LEFT, 100, 100, False)
home_video_btn.add_event_cb(lambda e: home_video_btn_clicked_1_event_cb(e,video), lv.EVENT.CLICKED, None)


def video_imgbtn_1_clicked_1_event_cb(e,home):
    src = e.get_target()
    code = e.get_code()
    lv.scr_load_anim(home, lv.SCR_LOAD_ANIM.OVER_RIGHT, 100, 100, False)
video_imgbtn_1.add_event_cb(lambda e: video_imgbtn_1_clicked_1_event_cb(e,home), lv.EVENT.CLICKED, None)


def home_backup_btn_clicked_1_event_cb(e,back_up):
    src = e.get_target()
    code = e.get_code()
    lv.scr_load_anim(back_up, lv.SCR_LOAD_ANIM.OVER_LEFT, 100, 100, False)
home_backup_btn.add_event_cb(lambda e: home_backup_btn_clicked_1_event_cb(e,back_up), lv.EVENT.CLICKED, None)


def home_music_btn_clicked_1_event_cb(e,music):
    src = e.get_target()
    code = e.get_code()
    lv.scr_load_anim(music, lv.SCR_LOAD_ANIM.OVER_LEFT, 100, 100, False)
home_music_btn.add_event_cb(lambda e: home_music_btn_clicked_1_event_cb(e,music), lv.EVENT.CLICKED, None)


def music_music_return_btn_clicked_1_event_cb(e,home):
    src = e.get_target()
    code = e.get_code()
    lv.scr_load_anim(home, lv.SCR_LOAD_ANIM.OVER_RIGHT, 100, 100, False)
music_music_return_btn.add_event_cb(lambda e: music_music_return_btn_clicked_1_event_cb(e,home), lv.EVENT.CLICKED, None)



# content from custom.py

# Load the default screen
lv.scr_load(home)

while SDL.check():
    time.sleep_ms(5)
