function UI.removeTableRow(ui,widget,index)
    local rows=simExtUI_getRowCount(ui,widget)
    local cols=simExtUI_getColumnCount(ui,widget)
    for row=index,rows-2 do
        for col=0,cols-1 do
            print(string.format('removeTableRow(%d,%d,%d) -- (%d,%d)',ui,widget,index,row,col))
            simExtUI_setItem(ui,widget,row,col,simExtUI_getItem(ui,widget,row+1,col))
        end
    end
    simExtUI_setRowCount(ui,widget,rows-1)
end

function UI.removeTableColumn(ui,widget,index)
    local rows=simExtUI_getRowCount(ui,widget)
    local cols=simExtUI_getColumnCount(ui,widget)
    for col=index,cols-2 do
        for row=0,rows-1 do
            print(string.format('removeTableColumn(%d,%d,%d) -- (%d,%d)',ui,widget,index,row,col))
            simExtUI_setItem(ui,widget,row,col,simExtUI_getItem(ui,widget,row,col+1))
        end
    end
    simExtUI_setColumnCount(ui,widget,cols-1)
end

