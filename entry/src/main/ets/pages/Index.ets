import { getNativeBuildInfo, runNativeSelfTest } from 'libstellarcanvas.so';

@Entry
@Component
struct Index {
  @State private nativeStatus: string = '正在检查原生运行时…';
  @State private nativeReady: boolean = false;
  @State private activeTool: number = 2;
  @State private activeLayer: number = 0;
  @State private currentColor: string = '#2C87D3';

  private readonly menus: string[] = [
    '文件', '编辑', '视图', '图像', '图层', '选择', '滤镜', '工具', '设置', '窗口', '帮助'
  ];
  private readonly toolSymbols: string[] = [
    '➤', '⌖', '✎', '╱', '▭', '◯', '⌁', 'T', '▱', '⌗', '⌕', '✥', '▧', '⌫', '◇', '▾'
  ];
  private readonly toolNames: string[] = [
    '选择形状', '移动工具', '自由手画笔', '直线工具', '矩形工具', '椭圆工具',
    '贝塞尔曲线', '文本工具', '裁剪工具', '渐变工具', '缩放工具', '平移工具',
    '多边形工具', '连续区域选择', '参考图像', '更多工具'
  ];
  private readonly swatches: string[] = [
    '#111827', '#F8FAFC', '#E5484D', '#F59E0B', '#FDE047', '#55B985',
    '#2C87D3', '#6550B9', '#C653B9', '#885A3A', '#94A3B8', '#374151'
  ];
  private readonly layerNames: string[] = ['高光与细节', '角色颜色', '线稿', '背景'];
  private readonly layerColors: string[] = ['#6AB4E8', '#ED7D72', '#4B5563', '#D8C6A5'];

  aboutToAppear(): void {
    this.checkNativeRuntime();
  }

  private checkNativeRuntime(): void {
    try {
      const passed = runNativeSelfTest();
      this.nativeStatus = `${getNativeBuildInfo()} · 自检${passed ? '通过' : '失败'}`;
      this.nativeReady = passed;
    } catch (error) {
      this.nativeStatus = `原生库加载失败：${String(error)}`;
      this.nativeReady = false;
    }
  }

  @Builder
  private DividerLine() {
    Divider()
      .vertical(true)
      .height(22)
      .color('#43454A')
      .margin({ left: 5, right: 5 })
  }

  @Builder
  private IconButton(symbol: string, selected: boolean = false) {
    Button({ type: ButtonType.Normal }) {
      Text(symbol)
        .fontSize(16)
        .fontColor(selected ? '#FFFFFF' : '#D1D5DB')
    }
    .width(34)
    .height(32)
    .padding(0)
    .backgroundColor(selected ? '#326F9F' : Color.Transparent)
    .borderRadius(4)
    .hoverEffect(HoverEffect.Auto)
  }

  @Builder
  private SectionTitle(title: string, suffix: string = '') {
    Row() {
      Text(title)
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .fontColor('#ECEDEE')
      Blank()
      if (suffix.length > 0) {
        Text(suffix)
          .fontSize(15)
          .fontColor('#AEB1B5')
      }
    }
    .width('100%')
    .height(34)
    .padding({ left: 10, right: 9 })
    .backgroundColor('#303236')
    .border({ width: { bottom: 1 }, color: '#1F2023' })
  }

  @Builder
  private TitleBar() {
    Row() {
      Stack({ alignContent: Alignment.Center }) {
        Circle({ width: 24, height: 24 })
          .fill('#7458B3')
        Text('K')
          .fontSize(14)
          .fontWeight(FontWeight.Bold)
          .fontColor('#FFFFFF')
      }
      .margin({ right: 9 })

      Text('未命名.kra (16.0 MiB) — StellarCanvas')
        .fontSize(13)
        .fontColor('#E7E8EA')

      Blank()

      Text('—')
        .fontSize(18)
        .fontColor('#C8C9CC')
        .width(44)
        .textAlign(TextAlign.Center)
      Text('□')
        .fontSize(15)
        .fontColor('#C8C9CC')
        .width(44)
        .textAlign(TextAlign.Center)
      Text('×')
        .fontSize(18)
        .fontColor('#C8C9CC')
        .width(44)
        .textAlign(TextAlign.Center)
    }
    .width('100%')
    .height(36)
    .padding({ left: 10 })
    .backgroundColor('#252629')
  }

  @Builder
  private MenuBar() {
    Row() {
      ForEach(this.menus, (item: string) => {
        Text(item)
          .fontSize(13)
          .fontColor('#E1E2E4')
          .height(31)
          .padding({ left: 10, right: 10 })
          .textAlign(TextAlign.Center)
      })
      Blank()
    }
    .width('100%')
    .height(32)
    .backgroundColor('#303236')
    .border({ width: { bottom: 1 }, color: '#1A1B1D' })
  }

  @Builder
  private MainToolbar() {
    Row({ space: 3 }) {
      this.IconButton('▤')
      this.IconButton('⌄')
      this.IconButton('▣')
      this.DividerLine()
      this.IconButton('↶')
      this.IconButton('↷')
      this.DividerLine()
      this.IconButton('✂')
      this.IconButton('▥')
      this.IconButton('▧')
      this.DividerLine()

      Text('Basic-5 Size Opacity')
        .fontSize(12)
        .fontColor('#D4D6D8')
        .height(28)
        .width(158)
        .padding({ left: 10 })
        .backgroundColor('#252629')
        .borderRadius(3)

      Text('大小：')
        .fontSize(12)
        .fontColor('#D4D6D8')
        .margin({ left: 7 })
      Row() {
        Text('40.00 px')
          .fontSize(12)
          .fontColor('#E3E4E6')
          .padding({ left: 8 })
        Blank()
        Text('▴▾')
          .fontSize(9)
          .fontColor('#A8AAAE')
          .padding({ right: 5 })
      }
      .width(100)
      .height(28)
      .backgroundColor('#252629')
      .borderRadius(3)

      Text('不透明度：')
        .fontSize(12)
        .fontColor('#D4D6D8')
        .margin({ left: 7 })
      Row() {
        Text('100%')
          .fontSize(12)
          .fontColor('#E3E4E6')
          .padding({ left: 8 })
        Blank()
        Text('▴▾')
          .fontSize(9)
          .fontColor('#A8AAAE')
          .padding({ right: 5 })
      }
      .width(82)
      .height(28)
      .backgroundColor('#252629')
      .borderRadius(3)

      Blank()
      this.IconButton('◈', true)
      this.IconButton('▦')
      this.IconButton('≡')
    }
    .width('100%')
    .height(42)
    .padding({ left: 7, right: 8 })
    .backgroundColor('#37393D')
    .border({ width: { bottom: 1 }, color: '#1A1B1D' })
  }

  @Builder
  private Toolbox() {
    Column() {
      Grid() {
        ForEach(this.toolSymbols, (symbol: string, index: number) => {
          GridItem() {
            Button({ type: ButtonType.Normal }) {
              Text(symbol)
                .fontSize(index === 2 ? 21 : 17)
                .fontColor(this.activeTool === index ? '#FFFFFF' : '#D3D5D8')
            }
            .width(35)
            .height(35)
            .padding(0)
            .backgroundColor(this.activeTool === index ? '#3477A9' : Color.Transparent)
            .borderRadius(3)
            .onClick(() => {
              this.activeTool = index;
            })
          }
        })
      }
      .columnsTemplate('1fr 1fr')
      .columnsGap(2)
      .rowsGap(2)
      .padding({ top: 5, left: 4, right: 4 })
      .layoutWeight(1)

      Divider().color('#44464B')
      Text(this.toolNames[this.activeTool])
        .fontSize(10)
        .fontColor('#B8BBC0')
        .maxLines(2)
        .textAlign(TextAlign.Center)
        .padding({ left: 3, right: 3, top: 7, bottom: 7 })
    }
    .width(79)
    .height('100%')
    .backgroundColor('#303236')
    .border({ width: { right: 1 }, color: '#1A1B1D' })
  }

  @Builder
  private RulerTop() {
    Row() {
      ForEach(['0', '200', '400', '600', '800', '1000', '1200'], (mark: string) => {
        Row() {
          Divider().vertical(true).height(6).color('#85888D')
          Text(mark).fontSize(8).fontColor('#A7A9AD').margin({ left: 2 })
        }
        .layoutWeight(1)
      })
    }
    .width('100%')
    .height(17)
    .padding({ left: 18, right: 12 })
    .backgroundColor('#2A2C2F')
  }

  @Builder
  private Artwork() {
    Stack({ alignContent: Alignment.Center }) {
      Rect()
        .width('100%')
        .height('100%')
        .fill('#F4EFE6')

      Circle({ width: '47%', height: '63%' })
        .fill('#E8C5A8')
        .offset({ x: 38, y: 3 })

      Circle({ width: '42%', height: '56%' })
        .fill('#25364F')
        .offset({ x: 36, y: -35 })

      Ellipse()
        .width('33%')
        .height('43%')
        .fill('#F3C9AA')
        .offset({ x: 45, y: -10 })

      Ellipse()
        .width('25%')
        .height('9%')
        .fill('#2C87D3')
        .rotate({ angle: -13 })
        .offset({ x: -155, y: 110 })

      Ellipse()
        .width('30%')
        .height('12%')
        .fill('#E5484D')
        .rotate({ angle: 16 })
        .offset({ x: 125, y: 145 })

      Circle({ width: 31, height: 31 })
        .fill('#F8FAFC')
        .offset({ x: 16, y: -21 })
      Circle({ width: 12, height: 12 })
        .fill('#1E293B')
        .offset({ x: 17, y: -20 })

      Text('KRITA')
        .fontSize(31)
        .fontWeight(FontWeight.Bold)
        .fontColor('#263244')
        .letterSpacing(5)
        .offset({ x: -172, y: -142 })
      Text('创作，自由表达')
        .fontSize(13)
        .fontColor('#536273')
        .letterSpacing(2)
        .offset({ x: -171, y: -108 })

      Row({ space: 6 }) {
        Circle({ width: 8, height: 8 }).fill('#2C87D3')
        Circle({ width: 8, height: 8 }).fill('#E5484D')
        Circle({ width: 8, height: 8 }).fill('#F59E0B')
      }
      .offset({ x: -167, y: -77 })
    }
    .width('72%')
    .height('80%')
    .backgroundColor('#F4EFE6')
    .shadow({ radius: 15, color: '#85000000', offsetX: 0, offsetY: 8 })
  }

  @Builder
  private CanvasArea() {
    Column() {
      this.RulerTop()

      Stack({ alignContent: Alignment.Center }) {
        Rect()
          .width('100%')
          .height('100%')
          .fill('#202124')
        this.Artwork()
      }
      .layoutWeight(1)
      .width('100%')

      Row() {
        Row({ space: 8 }) {
          Text('＋').fontSize(13).fontColor('#C7C9CC')
          Text('未命名.kra').fontSize(12).fontColor('#F0F1F2')
          Text('×').fontSize(13).fontColor('#AEB0B4')
        }
        .height(30)
        .padding({ left: 12, right: 10 })
        .backgroundColor('#37393D')
        Blank()
      }
      .width('100%')
      .height(31)
      .backgroundColor('#292B2E')
      .border({ width: { top: 1 }, color: '#17181A' })
    }
    .layoutWeight(1)
    .height('100%')
    .backgroundColor('#202124')
  }

  @Builder
  private ColorSelector() {
    Column() {
      this.SectionTitle('高级颜色选择器', '⌃')
      Stack({ alignContent: Alignment.Center }) {
        Circle({ width: 154, height: 154 })
          .fill('#2C87D3')
          .border({ width: 16, color: '#E5484D' })
        Circle({ width: 112, height: 112 })
          .fill('#303236')
          .border({ width: 8, color: '#F59E0B' })
        Stack({ alignContent: Alignment.Center }) {
          Rect()
            .width(58)
            .height(58)
            .fill('#F8FAFC')
            .rotate({ angle: 45 })
          Rect()
            .width(39)
            .height(39)
            .fill(this.currentColor)
            .rotate({ angle: 45 })
            .offset({ x: -3, y: 4 })
        }
        Circle({ width: 13, height: 13 })
          .fill('#FFFFFF')
          .border({ width: 2, color: '#1F2937' })
          .offset({ x: 49, y: -55 })
      }
      .width('100%')
      .height(182)
      .backgroundColor('#292B2E')

      Grid() {
        ForEach(this.swatches, (color: string) => {
          GridItem() {
            Button({ type: ButtonType.Normal })
              .width(25)
              .height(22)
              .padding(0)
              .backgroundColor(color)
              .border({ width: this.currentColor === color ? 2 : 1,
                color: this.currentColor === color ? '#FFFFFF' : '#1C1D1F' })
              .borderRadius(2)
              .onClick(() => {
                this.currentColor = color;
              })
          }
        })
      }
      .columnsTemplate('repeat(6, 1fr)')
      .columnsGap(4)
      .rowsGap(4)
      .height(56)
      .padding({ left: 9, right: 9, top: 3, bottom: 5 })
      .backgroundColor('#292B2E')
    }
    .width('100%')
  }

  @Builder
  private BrushPresets() {
    Column() {
      this.SectionTitle('画笔预设', '⌃')
      Grid() {
        ForEach(['●', '◉', '✦', '╱', '≈', '✧', '⬟', '▰'], (brush: string, index: number) => {
          GridItem() {
            Column({ space: 3 }) {
              Stack({ alignContent: Alignment.Center }) {
                Rect()
                  .width(48)
                  .height(42)
                  .fill(index === 0 ? '#465B6A' : '#3A3C40')
                  .borderRadius(4)
                Text(brush)
                  .fontSize(index === 3 ? 24 : 19)
                  .fontColor(index % 3 === 0 ? '#C7DDF0' : '#D6D8DA')
              }
              Text(index === 0 ? 'Basic-5' : `画笔 ${index + 1}`)
                .fontSize(8)
                .fontColor('#B9BBC0')
                .maxLines(1)
            }
          }
        })
      }
      .columnsTemplate('repeat(4, 1fr)')
      .columnsGap(4)
      .rowsGap(7)
      .height(132)
      .padding({ left: 8, right: 8, top: 8 })
      .backgroundColor('#292B2E')
    }
    .width('100%')
  }

  @Builder
  private Layers() {
    Column() {
      this.SectionTitle('图层', '⌃')
      Row({ space: 5 }) {
        Text('正常').fontSize(11).fontColor('#D7D8DA')
        Blank()
        Text('不透明度 100%').fontSize(10).fontColor('#AEB0B4')
      }
      .height(28)
      .padding({ left: 9, right: 9 })
      .backgroundColor('#303236')

      ForEach(this.layerNames, (name: string, index: number) => {
        Row({ space: 8 }) {
          Text(index === 3 ? '○' : '◉')
            .fontSize(11)
            .fontColor('#C8CACD')
          Rect()
            .width(34)
            .height(28)
            .fill(this.layerColors[index])
            .border({ width: 1, color: '#17181A' })
          Column() {
            Text(name)
              .fontSize(11)
              .fontColor('#E3E4E6')
              .maxLines(1)
            Text(index === 3 ? '背景图层' : '绘画图层')
              .fontSize(8)
              .fontColor('#95989D')
          }
          .alignItems(HorizontalAlign.Start)
          Blank()
          Text(index === 3 ? '▣' : 'α')
            .fontSize(10)
            .fontColor('#9FA2A6')
        }
        .width('100%')
        .height(40)
        .padding({ left: 8, right: 8 })
        .backgroundColor(this.activeLayer === index ? '#3C5365' : '#292B2E')
        .border({ width: { bottom: 1 }, color: '#202124' })
        .onClick(() => {
          this.activeLayer = index;
        })
      })

      Row({ space: 13 }) {
        Text('＋').fontSize(18).fontColor('#D7D9DB')
        Text('▣').fontSize(13).fontColor('#C7C9CC')
        Text('⌄').fontSize(13).fontColor('#C7C9CC')
        Blank()
        Text('⌫').fontSize(14).fontColor('#C7C9CC')
        Text('≡').fontSize(14).fontColor('#C7C9CC')
      }
      .height(31)
      .padding({ left: 10, right: 10 })
      .backgroundColor('#303236')
    }
    .width('100%')
    .layoutWeight(1)
  }

  @Builder
  private RightDockers() {
    Column() {
      this.ColorSelector()
      Divider().color('#17181A').strokeWidth(3)
      this.BrushPresets()
      Divider().color('#17181A').strokeWidth(3)
      this.Layers()
    }
    .width(286)
    .height('100%')
    .backgroundColor('#292B2E')
    .border({ width: { left: 1 }, color: '#17181A' })
  }

  @Builder
  private StatusBar() {
    Row() {
      Circle({ width: 7, height: 7 })
        .fill(this.nativeReady ? '#55C69A' : '#EF6B73')
      Text(this.nativeStatus)
        .fontSize(9)
        .fontColor('#AEB1B5')
        .maxLines(1)
        .textOverflow({ overflow: TextOverflow.Ellipsis })
        .margin({ left: 6 })
        .constraintSize({ maxWidth: 330 })
      Blank()
      Text('RGB/Alpha (8-bit integer/channel)')
        .fontSize(9)
        .fontColor('#B2B4B8')
      this.DividerLine()
      Text('1,920 × 1,080 (16.0 MiB)')
        .fontSize(9)
        .fontColor('#B2B4B8')
      this.DividerLine()
      Text('画布旋转：0.0°')
        .fontSize(9)
        .fontColor('#B2B4B8')
      this.DividerLine()
      Text('33.3%')
        .fontSize(9)
        .fontColor('#E0E1E3')
      Row() {
        Rect().width('33%').height(3).fill('#579ACB')
      }
      .width(118)
      .height(4)
      .backgroundColor('#1E2022')
      .margin({ left: 7, right: 7 })
      Text('＋')
        .fontSize(13)
        .fontColor('#D3D5D8')
    }
    .width('100%')
    .height(25)
    .padding({ left: 8, right: 10 })
    .backgroundColor('#303236')
    .border({ width: { top: 1 }, color: '#18191B' })
  }

  build() {
    Column() {
      this.TitleBar()
      this.MenuBar()
      this.MainToolbar()

      Row() {
        this.Toolbox()
        this.CanvasArea()
        this.RightDockers()
      }
      .layoutWeight(1)
      .width('100%')

      this.StatusBar()
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#202124')
  }
}
