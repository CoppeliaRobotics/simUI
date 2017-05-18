local simUI={}

--@fun insertTableRow
--@arg ui the ui handle
--@arg widget the widget identifier
--@arg index the index (0-based) where the new row will appear
function simUI.insertTableRow(ui,widget,index)
    local rows=simExtUI_getRowCount(ui,widget)
    local cols=simExtUI_getColumnCount(ui,widget)
    simExtUI_setRowCount(ui,widget,rows+1)
    for row=rows-1,index+1,-1 do
        for col=0,cols-1 do
            simExtUI_setItem(ui,widget,row,col,simExtUI_getItem(ui,widget,row-1,col))
        end
    end
end

--@fun removeTableRow
--@arg ui the ui handle
--@arg widget the widget identifier
--@arg index the row index (0-based) to remove
function simUI.removeTableRow(ui,widget,index)
    local rows=simExtUI_getRowCount(ui,widget)
    local cols=simExtUI_getColumnCount(ui,widget)
    for row=index,rows-2 do
        for col=0,cols-1 do
            simExtUI_setItem(ui,widget,row,col,simExtUI_getItem(ui,widget,row+1,col))
        end
    end
    simExtUI_setRowCount(ui,widget,rows-1)
end

--@fun insertTableColumn
--@arg ui the ui handle
--@arg widget the widget identifier
--@arg index the index (0-based) where the new column will appear
function simUI.insertTableColumn(ui,widget,index)
    local rows=simExtUI_getRowCount(ui,widget)
    local cols=simExtUI_getColumnCount(ui,widget)
    simExtUI_setColumnCount(ui,widget,cols+1)
    for col=cols-1,index+1,-1 do
        for row=0,rows-1 do
            simExtUI_setItem(ui,widget,row,col,simExtUI_getItem(ui,widget,row,col-1))
        end
    end
end

--@fun removeTableColumn
--@arg ui the ui handle
--@arg widget the widget identifier
--@arg index the column index (0-based) to remove
function simUI.removeTableColumn(ui,widget,index)
    local rows=simExtUI_getRowCount(ui,widget)
    local cols=simExtUI_getColumnCount(ui,widget)
    for col=index,cols-2 do
        for row=0,rows-1 do
            simExtUI_setItem(ui,widget,row,col,simExtUI_getItem(ui,widget,row,col+1))
        end
    end
    simExtUI_setColumnCount(ui,widget,cols-1)
end

return simUI
