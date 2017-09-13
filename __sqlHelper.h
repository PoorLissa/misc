#pragma once
#ifndef SQL_HELPER_H
#define SQL_HELPER_H

/*
    How to Use:
 -------------------------------- cut -----------------------------------------

    using sqlHelper::sql;

    sql _sql, qh1;

    _sql.SELECT("obj_name1, obj_name2")
        .FROM("table_name")
            .LEFT_JOIN("other_table_name")
                .ON("param = 1")
        .WHERE("id = 2")
            .AND("calc1 = 3")
                .AND("calc2 = 4")
        .ORDER_BY("id");

    AnsiString str = _sql;

    ==> str = " SELECT obj_name1, obj_name2 FROM table_name LEFT JOIN other_table_name ON param = 1 WHERE id = 2 AND calc1 = 3 AND calc2 = 4 ORDER BY id"

    qh1.SELECT(
        qh1.lB().SELECT("*").FROM("tab1").WHERE("id=1").rB()
      + qh1.SELECT("*").FROM("tab2").WHERE("id=2")
      + qh1.txt(" preved medved ")
      + qh1.lB().SELECT("imu_adr").FROM("BI_R_mzayav" ).WHERE("calc_id=%i", 170520).rB().AS("t1")
      + qh1.lB().SELECT("prem"   ).FROM("BI_R_objects").WHERE("calc_id=%i", 170523).AND("num=%i", 6).rB().AS("t2")
      + "StrSumm"
    )
    .FROM("BI_R_main").WHERE("calc_id=%i", 170523).GROUP_BY("group").ORDER_BY("order");

    == > SELECT (SELECT * FROM (tab1) WHERE id=1), SELECT * FROM (tab2) WHERE id=2,  preved medved , (SELECT imu_adr FROM (BI_R_mzayav) WHERE calc_id=170520) AS t1, (SELECT prem FROM (BI_R_objects) WHERE calc_id=170523 AND num=6) AS t2, StrSumm FROM (BI_R_main) WHERE calc_id=170523 GROUP BY group ORDER BY order

 -------------------------------- cut -----------------------------------------
*/

namespace sqlHelper
{
    #define  sql       qHelper
    #define  Str       AnsiString
    #define cStr const AnsiString
    #define cInt const int

    class qHelper;
    class qAny;
    class qSelect;
    class qUpdate;
    class qFrom;
    class qWhere;
    class qLeftJoin;
    class qRightJoin;    
    class qOn;
    class qAnd;
    class qOrderBy;
    class qGroupBy;
    class qSet;
    class qValues;
    class qAs;
    class qLBracket;
    class qRBracket;
    class qDeleteFrom;
    class qInsertInto;
    
    static AnsiString Commands[] = { "",    " ",   ",",   "(",  ")", "SELECT ", " FROM ", " WHERE ", " LEFT JOIN ", " RIGHT JOIN ", " ORDER BY ", " ON ", " AND ", " GROUP BY ", "UPDATE ", " SET ", "DELETE FROM ", "INSERT INTO ", " VALUES ", " AS " };
    enum   CommandsEnum            { Empty, Space,  Comma, lBr, rBr,  Select,     From,     Where,     LeftJoin,      RightJoin,      OrderBy,      On,     And,     GroupBy,     Update,     Set,    DeleteFrom,     InsertInto,      Values  ,   As   };

    

    class qCommandBase {
        qCommandBase& operator =(qCommandBase &);

     public:
        qCommandBase(qHelper *query, const AnsiString str) : _query(query), _str(str) {}
        virtual ~qCommandBase() = 0;

        // Перегружаем оператор преобразования к AnsiString, чтобы не заморачиваться с вызовом метода get(),
        // и чтобы каждый объект мог вернуть итоговую строку запроса
        inline operator AnsiString() const { return _str; }

        // --- Общие методы для всех субклассов ---

        // Добавляем произвольный текст
        qAny txt(const AnsiString str) { _str = Commands[Empty]; return QueryFunc<qAny>(this, str, Empty); }

     protected:

        // Главный метод. Достраивает строку, сохраняет ее и возвращает объект нужного типа.
        template<class Type>
        Type QueryFunc(qCommandBase *Query, const AnsiString Data, const int commandNo)
        {
            Type    *thisType = dynamic_cast<Type*>(Query);
            qHelper *mainObj  = Query->_query;

            return globalQueryFunc(thisType, mainObj, Data, commandNo);
        }

     protected:
        qHelper    *_query;
        AnsiString  _str;
    };

    qCommandBase::~qCommandBase()
    {
    }



    // Основной класс, через объект которого будем создавать запрос
    class qHelper : public qCommandBase {
        qHelper& operator =(qHelper &);
     public:
        qHelper() : qCommandBase(this, Commands[Empty]) {}

        // При вызове методов основного класса начинаем собирать новый запрос с пустой строки
        qSelect     SELECT      (const AnsiString str) { _str = Commands[Empty]; return QueryFunc< qSelect     >( this, str, Select     ); }
        qUpdate     UPDATE      (const AnsiString str) { _str = Commands[Empty]; return QueryFunc< qUpdate     >( this, str, Update     ); }
        qDeleteFrom DELETE_FROM (const AnsiString str) { _str = Commands[Empty]; return QueryFunc< qDeleteFrom >( this, str, DeleteFrom ); }
        qInsertInto INSERT_INTO (const AnsiString str) { _str = Commands[Empty]; return QueryFunc< qInsertInto >( this, str, InsertInto ); }
        qLBracket   lB          ()                     { _str = Commands[Empty]; return QueryFunc< qLBracket   >( this,_str, lBr        ); }

        inline void       set   (AnsiString str)       {  _str = str; }
        inline AnsiString get   () const               { return _str; }
    };



    // Промежуточный класс для суммирования объектов (через запятую или через пробел)
    class qSummable : public qCommandBase {
        qSummable& operator =(qSummable &);

     public:
        qSummable(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        friend qSummable operator +(const char *, qSummable);                   // Сложение через запятую
        friend qSummable operator &(const char *, qSummable);                   // Сложение через пробел

        qSummable operator +(qSummable other)
        {
            sum_viaComma(other._str);
            return *this;
        }

        qSummable operator +(AnsiString other)
        {
            sum_viaComma(other);
            return *this;
        }

        qSummable operator &(qSummable other)
        {
            sum_viaSpace(other._str);
            return *this;
        }

        qSummable operator &(AnsiString other)
        {
            sum_viaSpace(other);
            return *this;
        }

     private:
        inline void sum_viaComma(AnsiString &str)
        {
            _str += Commands[Comma] + Commands[Space] + str;
        }

        inline void sum_viaSpace(AnsiString &str)
        {
            _str += Commands[Space] + str;
        }
    };

    qSummable operator +(const char *str, qSummable other)
    {
        other._str = AnsiString(str) + Commands[Comma] + Commands[Space] + other._str;
        return other;
    }

    qSummable operator &(const char *str, qSummable other)
    {
        other._str = AnsiString(str) + Commands[Space] + other._str;
        return other;
    }



    // Вынес повторяющееся семейство методов WHERE в отдельный класс
    class WhereFamily : public qSummable {
        WhereFamily& operator =(WhereFamily &);
     public:
        WhereFamily(qHelper *query, const AnsiString str) : qSummable(query, str) {}

        qWhere WHERE (cStr str       ) { return QueryFunc<qWhere>(this, str,                         Where); }
        qWhere WHERE (Str str, cInt i) { return QueryFunc<qWhere>(this, str.sprintf(str.c_str(), i), Where); }   // %i
        qWhere WHERE (Str str, cStr s) { return QueryFunc<qWhere>(this, str.sprintf(str.c_str(), s), Where); }   // %s
    };



    // Вынес повторяющееся семейство методов AND в отдельный класс
    class AndFamily : public qSummable {
        AndFamily& operator =(AndFamily &);
     public:
        AndFamily(qHelper *query, const AnsiString str) : qSummable(query, str) {}

        qAnd AND (const AnsiString str) { return QueryFunc<qAnd>(this, str,                         And); }
        qAnd AND (Str str, cInt i     ) { return QueryFunc<qAnd>(this, str.sprintf(str.c_str(), i), And); }      // %i
        qAnd AND (Str str, cStr s     ) { return QueryFunc<qAnd>(this, str.sprintf(str.c_str(), s), And); }      // %s
    };



    class qSelect : public qCommandBase {
        qSelect& operator =(qSelect &);
     public:
        qSelect(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        qFrom FROM (const AnsiString str) { return QueryFunc<qFrom>(this, str, From ); }
        qAs   AS   (const AnsiString str) { return QueryFunc<qAs  >(this, str, As   ); }
    };



    class qAny : public qSummable {
        qAny& operator =(qAny &);
     public:
        qAny(qHelper *query, const AnsiString str) : qSummable(query, str) {}

        // Добавить методы по мере необходимости
        qSelect SELECT  (const AnsiString str) { return QueryFunc<qSelect>(this, str, Select); }
        qFrom   FROM    (const AnsiString str) { return QueryFunc<qFrom  >(this, str, From  ); }
        qAs     AS      (const AnsiString str) { return QueryFunc<qAs    >(this, str, As    ); }
    };



    class qAs : public qSummable {
        qAs& operator =(qAs &);
     public:
        qAs(qHelper *query, const AnsiString str) : qSummable(query, str) {}

        qFrom FROM (const AnsiString str) { return QueryFunc<qFrom>(this, str, From); }
        qAs   AS   (const AnsiString str) { return QueryFunc<qAs  >(this, str, As  ); }
    };



    class qUpdate : public qCommandBase {
        qUpdate& operator =(qUpdate &);
     public:
        qUpdate(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        qSet SET(const AnsiString str) { return QueryFunc<qSet>(this, str,                         Set); }
        qSet SET(Str str, cInt n     ) { return QueryFunc<qSet>(this, str.sprintf(str.c_str(), n), Set); }
        qSet SET(Str str, cStr s     ) { return QueryFunc<qSet>(this, str.sprintf(str.c_str(), s), Set); }
    };



    class qDeleteFrom : public WhereFamily {
        qDeleteFrom& operator =(qDeleteFrom &);
     public:
        qDeleteFrom(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}
    };



    class qInsertInto : public qCommandBase {
        qInsertInto& operator =(qInsertInto &);
     public:
        qInsertInto(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        qValues VALUES(const AnsiString str) { return QueryFunc<qValues>(this, str, Values); }
    };



    class qValues : public qCommandBase {
        qValues& operator =(qValues &);
     public:
        qValues(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}
    };



    class qFrom : public WhereFamily {
        qFrom& operator =(qFrom &);
     public:
        qFrom(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}

        qLeftJoin  LEFT_JOIN  (const AnsiString str) { return QueryFunc<qLeftJoin >(this, str, LeftJoin ); }
        qRightJoin RIGHT_JOIN (const AnsiString str) { return QueryFunc<qRightJoin>(this, str, RightJoin); }
        qOrderBy   ORDER_BY   (const AnsiString str) { return QueryFunc<qOrderBy  >(this, str, OrderBy  ); }
        qGroupBy   GROUP_BY   (const AnsiString str) { return QueryFunc<qGroupBy  >(this, str, GroupBy  ); }

        qFrom operator [] (AnsiString str) { return QueryFunc<qFrom>(this, str, From); }
    };



    class qSet : public WhereFamily {
        qSet& operator =(qSet &);
     public:
        qSet(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}
    };



    class qLeftJoin : public WhereFamily {
        qLeftJoin& operator =(qLeftJoin &);
     public:
        qLeftJoin(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}

        qOn      ON       (const AnsiString str) { return QueryFunc<qOn     >(this, str, On     ); }
        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(this, str, OrderBy); }
    };



    class qRightJoin : public WhereFamily {
        qRightJoin& operator =(qRightJoin &);
     public:
        qRightJoin(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}

        qOn      ON       (const AnsiString str) { return QueryFunc<qOn     >(this, str, On     ); }
        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(this, str, OrderBy); }
    };



    class qOn : public WhereFamily {
        qOn& operator =(qOn &);
     public:
        qOn(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}

        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(this, str, OrderBy); }
    };



    class qWhere : public AndFamily {
        qWhere& operator =(qWhere &);
     public:
        qWhere(qHelper *query, const AnsiString str) : AndFamily(query, str) {}

        qOrderBy  ORDER_BY (const AnsiString str) {                         return QueryFunc<qOrderBy >(this, str, OrderBy); }
        qGroupBy   GROUP_BY(const AnsiString str) {                         return QueryFunc<qGroupBy >(this, str, GroupBy); }        
        qRBracket rB       ()                     { _str = Commands[Empty]; return QueryFunc<qRBracket>(this, _str, rBr   ); }
    };



    class qAnd : public AndFamily {
        qAnd& operator =(qAnd &);
     public:
        qAnd(qHelper *query, const AnsiString str) : AndFamily(query, str) {}

        qOrderBy  ORDER_BY (const AnsiString str) {                         return QueryFunc<qOrderBy >(this, str, OrderBy); }
        qRBracket rB       ()                     { _str = Commands[Empty]; return QueryFunc<qRBracket>(this, _str, rBr   ); }
    };



    class qOrderBy : public qCommandBase {
        qOrderBy& operator =(qOrderBy &);
     public:
        qOrderBy(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}
    };



    class qGroupBy : public qCommandBase {
        qGroupBy& operator =(qGroupBy &);
     public:
        qGroupBy(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(this, str, OrderBy); }
    };



    class qLBracket : public qSummable {
        qLBracket& operator =(qLBracket &);
     public:
        qLBracket(qHelper *query, const AnsiString str) : qSummable(query, str) {}

        qSelect     SELECT      (const AnsiString str) { _str = Commands[Empty]; return QueryFunc<qSelect    >(this, str, Select    ); }
        qUpdate     UPDATE      (const AnsiString str) { _str = Commands[Empty]; return QueryFunc<qUpdate    >(this, str, Update    ); }
        qDeleteFrom DELETE_FROM (const AnsiString str) { _str = Commands[Empty]; return QueryFunc<qDeleteFrom>(this, str, DeleteFrom); }
        qInsertInto INSERT_INTO (const AnsiString str) { _str = Commands[Empty]; return QueryFunc<qInsertInto>(this, str, InsertInto); }
    };



    class qRBracket : public qSummable {
        qRBracket& operator =(qRBracket &);
     public:
        qRBracket(qHelper *query, const AnsiString str) : qSummable(query, str) {}

        qAs AS (const AnsiString str) { return QueryFunc<qAs>(this, str, As); }
    };

    // -------------- Глобальные шаблонные функции ----------------------------

    // Заключить строку данных в круглые скобки
    template <class Type>
    Type global_embrace_in_brackets(Type *Query, qHelper *helper, const AnsiString *Data, const int cmd)
    {
        helper->set(helper->get() + Commands[cmd] + Commands[lBr] + *Data + Commands[rBr]);

        return Type(helper, helper->get());
    }

    // Достроить строку данных - основной метод
    template <class Type>
    Type globalQueryFunc(Type *Query, qHelper *helper, const AnsiString Data, const int cmd)
    {
        helper->set(helper->get() + Commands[cmd] + Data);

        return Type(helper, helper->get());
    }

    // Специализированная версия
    template <>
    qFrom globalQueryFunc(qFrom *Query, qHelper *helper, const AnsiString Data, const int cmd)
    {
        return global_embrace_in_brackets(Query, helper, &Data, cmd);
    }

    // Специализированная версия
    template <>
    qValues globalQueryFunc(qValues *Query, qHelper *helper, const AnsiString Data, const int cmd)
    {
        return global_embrace_in_brackets(Query, helper, &Data, cmd);
    }

    // ------------------------------------------------------------------------
}

#endif

